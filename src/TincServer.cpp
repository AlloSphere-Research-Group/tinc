#include "tinc/TincServer.hpp"
#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDFData.hpp"
#include "tinc/ProcessorAsyncWrapper.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorGraph.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;
using namespace tinc_protobuf;

TincServer::TincServer() {
  mVersion = TINC_PROTOCOL_VERSION;
  mRevision = TINC_PROTOCOL_REVISION;
}

bool TincServer::processIncomingMessage(al::Message &message, al::Socket *src) {

  markBusy();
  while (message.remainingBytes() > 8) {
    size_t msgSize;
    memcpy(&msgSize, message.data(), sizeof(size_t));
    if (msgSize > message.remainingBytes()) {
      break;
    }
    message.pushReadIndex(8);
    // if (verbose()) {
    //   std::cout << "Server got " << msgSize << " of "
    //             << message.remainingBytes() << std::endl;
    // }
    google::protobuf::io::ArrayInputStream ais(message.data(), msgSize);
    google::protobuf::io::CodedInputStream codedStream(&ais);
    TincMessage tincMessage;
    if (tincMessage.ParseFromCodedStream(&codedStream)) {
      auto type = tincMessage.messagetype();
      auto objectType = tincMessage.objecttype();
      auto details = tincMessage.details();

      switch (type) {
      case MessageType::REQUEST:
        if (details.Is<ObjectId>()) {
          ObjectId objectId;
          details.UnpackTo(&objectId);
          readRequestMessage(objectType, objectId.id(), src);
        } else {
          std::cerr << __FUNCTION__ << ": Request message has invalid payload"
                    << std::endl;
        }
        break;
      case MessageType::REMOVE:
        if (verbose()) {
          std::cout << "Server received Remove message" << std::endl;
        }
        if (!readRemoveMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing remove message"
                    << std::endl;
        }
        break;
      case MessageType::REGISTER:
        if (verbose()) {
          std::cout << "Server received Register message" << std::endl;
        }
        if (!readRegisterMessage(objectType, (void *)&details, src, true)) {
          std::cerr << __FUNCTION__ << ": Error processing Register message"
                    << std::endl;
        } else {
        }
        break;
      case MessageType::CONFIGURE:
        if (!readConfigureMessage(objectType, (void *)&details, src, true)) {
          std::cerr << __FUNCTION__ << ": Error processing Configure message"
                    << std::endl;
        }
        break;
      case MessageType::COMMAND:
        if (verbose()) {
          std::cout << "Server received Command message" << std::endl;
        }
        if (!readCommandMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Command message"
                    << std::endl;
        }
        break;
      case MessageType::PING:

        if (verbose()) {
          std::cout << "Server received Command message" << std::endl;
        }
        if (!readPingMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Command message"
                    << std::endl;
        }
        break;
      case MessageType::PONG:
        std::cerr << __FUNCTION__
                  << ": Pong message received, but not implemented"
                  << std::endl;
        break;
      case MessageType::BARRIER_REQUEST:
        std::cerr << __FUNCTION__ << ": Unsupported BARRIER_REQUEST in server"
                  << std::endl;
        break;
      case MessageType::BARRIER_ACK_LOCK:
        if (details.Is<Command>()) {
          Command objectId;
          details.UnpackTo(&objectId);
          processBarrierAckLock(src, objectId.message_id());
        } else {
          std::cerr << __FUNCTION__ << ": Invalid payload for BARRIER_REQUEST"
                    << std::endl;
        }
        break;
      case MessageType::BARRIER_UNLOCK:
        std::cerr << __FUNCTION__ << ": Unsupported BARRIER_UNLOCK in server"
                  << std::endl;
        break;
      case MessageType::GOODBYE:
        if (verbose()) {
          std::cout << "Goodbye from " << src->address() << ":" << src->port()
                    << std::endl;
        }
        disconnectClient(src);
        break;
      case MessageType::TINC_WORKING_PATH:
        if (verbose()) {
          std::cout << "Ignored TINC_WORKING_PATH from " << src->address()
                    << ":" << src->port() << std::endl;
        }
        break;

      case MessageType::TINC_CLIENT_METADATA:
        if (details.Is<ClientMetaData>()) {
          ClientMetaData metadata;
          details.UnpackTo(&metadata);
          processClientMetadataMessage((void *)&metadata, src);
        }
        //        if (verbose()) {
        //          std::cout << "Ignored TINC_WORKING_PATH from " <<
        //          src->address()
        //                    << ":" << src->port() << std::endl;
        //        }
        break;
      default:
        std::cerr << __FUNCTION__ << ": Invalid message type" << std::endl;
      }
    } else {
      std::cerr << __FUNCTION__ << ": Error parsing message" << std::endl;
    }

    message.pushReadIndex(msgSize);
  }

  // if (verbose()) {
  //   std::cout << "Server message buffer : " << message.remainingBytes()
  //             << std::endl;
  // }

  markAvailable();
  return true;
}

void TincServer::setVerbose(bool verbose) {
  CommandConnection::mVerbose = verbose;
  TincProtocol::mVerbose = verbose;
}

bool TincServer::barrier(uint32_t group, float timeoutsec) {
  std::cerr << __FUNCTION__ << " Enter server barrier " << std::endl;
  std::unique_lock<std::mutex> lk(mBarrierLock);
  TincMessage msg;
  msg.set_messagetype(MessageType::BARRIER_REQUEST);
  msg.set_objecttype(ObjectType::GLOBAL);
  Command details;
  auto currentConsecutive = mBarrierConsecutive;
  details.set_message_id(currentConsecutive);
  mBarrierConsecutive++;

  auto *commandDetails = msg.details().New();
  commandDetails->PackFrom(details);
  msg.set_allocated_details(commandDetails);
  {
    // Prepare for barrier acks
    std::unique_lock<std::mutex> lk(mBarrierAckLock);
    mBarrierAcks[currentConsecutive] = {};
  }

  std::vector<al::Socket *> barrierSentRequests;

  {
    std::unique_lock<std::mutex> lk(mConnectionsLock);
    for (auto connection : mServerConnections) {
      bool ret = sendProtobufMessage(&msg, connection.get());
      if (ret) {
        barrierSentRequests.push_back(connection.get());
      }
    }
  }

  std::cerr << __FUNCTION__ << " Server sent BARRIER_REQUEST "
            << currentConsecutive << std::endl;
  std::vector<al::Socket *> barrierRequestsPending = barrierSentRequests;
  int timems = 0;

  while ((timems < (timeoutsec * 1000) || timeoutsec == 0.0) &&
         barrierRequestsPending.size() != 0) {

    if (mBarrierAckLock.try_lock()) {
      for (auto barrierAck : mBarrierAcks[currentConsecutive]) {
        auto posToPop = barrierRequestsPending.begin();
        if ((posToPop = std::find(barrierRequestsPending.begin(),
                                  barrierRequestsPending.end(), barrierAck)) !=
            barrierRequestsPending.end()) {
          std::cout << "Barrier ACK lock:" << barrierAck->address() << ":"
                    << barrierAck->port() << std::endl;
          barrierRequestsPending.erase(posToPop);
          mBarrierAcks[currentConsecutive].erase(
              std::find(mBarrierAcks[currentConsecutive].begin(),
                        mBarrierAcks[currentConsecutive].end(), barrierAck));
          // TODO write quicker way to go through pending
          break;
        } else {
          std::cerr << "ERROR: Unexpected ACK lock from: "
                    << barrierAck->address() << ":" << barrierAck->port()
                    << std::endl;
        }
      }
      mBarrierAckLock.unlock();
    }
    std::this_thread::sleep_for(
        std::chrono::milliseconds(barrierWaitGranularTimeMs));
    timems += barrierWaitGranularTimeMs;
  }

  std::cerr << __FUNCTION__ << " Server received all BARRIER_ACK_LOCK "
            << std::endl;
  // All node have acknowledged lock. So now send order to peroceed.
  TincMessage msgUnlock;
  msgUnlock.set_messagetype(MessageType::BARRIER_UNLOCK);
  msgUnlock.set_objecttype(ObjectType::GLOBAL);
  Command unlockDetails;
  unlockDetails.set_message_id(currentConsecutive);

  auto *commandUnlockDetails = msgUnlock.details().New();
  commandUnlockDetails->PackFrom(unlockDetails);
  msgUnlock.set_allocated_details(commandUnlockDetails);

  for (auto *connection : barrierSentRequests) {
    bool ret = sendProtobufMessage(&msgUnlock, connection);
    if (!ret) {
      std::cerr << "ERROR sending unlock command to " << connection->address()
                << ":" << connection->port() << std::endl;
    }
  }

  {
    std::unique_lock<std::mutex> lk(mBarrierAckLock);
    mBarrierAcks.erase(currentConsecutive);
  }
  std::cerr << __FUNCTION__ << " Exit server barrier --------" << std::endl;
  return (timems >= (timeoutsec * 1000) || timeoutsec == 0.0);
}

void TincServer::markBusy() {
  TincMessage msg;
  msg.set_messagetype(MessageType::STATUS);
  msg.set_objecttype(ObjectType::GLOBAL);
  StatusMessage statusMsg;
  statusMsg.set_status(StatusTypes::BUSY);

  auto *statusDetails = msg.details().New();
  statusDetails->PackFrom(statusMsg);
  msg.set_allocated_details(statusDetails);
  sendTincMessage(&msg);
  TincProtocol::markBusy();
}

void TincServer::markAvailable() {
  TincMessage msg;
  msg.set_messagetype(MessageType::STATUS);
  msg.set_objecttype(ObjectType::GLOBAL);
  StatusMessage statusMsg;
  statusMsg.set_status(StatusTypes::AVAILABLE);

  auto *statusDetails = msg.details().New();
  statusDetails->PackFrom(statusMsg);
  msg.set_allocated_details(statusDetails);
  sendTincMessage(&msg);

  TincProtocol::markAvailable();
}

void TincServer::onConnection(al::Socket *newConnection) {

  {
    TincMessage msg;
    msg.set_messagetype(MessageType::STATUS);
    msg.set_objecttype(ObjectType::GLOBAL);
    StatusMessage statusMsg;
    {
      std::unique_lock<std::mutex> lk(mBusyCountLock);
      if (mBusyCount == 0) {
        statusMsg.set_status(StatusTypes::AVAILABLE);
      } else {
        statusMsg.set_status(StatusTypes::BUSY);
      }
    }

    auto *statusDetails = msg.details().New();
    statusDetails->PackFrom(statusMsg);
    msg.set_allocated_details(statusDetails);
    sendTincMessage(&msg, newConnection);
  }
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::TINC_WORKING_PATH);
    msg.set_objecttype(ObjectType::GLOBAL);
    TincPath path;
    path.set_path(al::File::currentPath());
    auto *statusDetails = msg.details().New();
    statusDetails->PackFrom(path);
    msg.set_allocated_details(statusDetails);
    sendTincMessage(&msg, newConnection);
  }
}

std::pair<std::string, uint16_t> TincServer::serverAddress() {
  return {mSocket.address(), mSocket.port()};
}

void TincServer::disconnectAllClients() {
  std::unique_lock<std::mutex> lk(mConnectionsLock);
  for (auto conn : mServerConnections) {
    conn->close();
  }
  mServerConnections.clear();
}

void TincServer::processBarrierAckLock(al::Socket *src,
                                       uint64_t barrierConsecutive) {
  std::cerr << __FUNCTION__ << " ACK_LOCK from " << src->address() << ":"
            << src->port() << std::endl;
  std::unique_lock<std::mutex> lk(mBarrierAckLock);
  if (mBarrierAcks.find(barrierConsecutive) != mBarrierAcks.end()) {
    mBarrierAcks[barrierConsecutive].push_back(src);
  } else {
    std::cerr << __FUNCTION__ << " ERROR unexpected ACK_LOCK. Ignoring"
              << std::endl;
  }
}

void TincServer::processClientMetadataMessage(void *metadata, al::Socket *src) {
  auto *data = static_cast<ClientMetaData *>(metadata);
  mClientHostnames[src] = data->clienthost();
}

void TincServer::disconnectClient(al::Socket *src) {
  std::unique_lock<std::mutex> lk(mConnectionsLock);
  for (auto connIt = mServerConnections.begin();
       connIt != mServerConnections.end(); connIt++) {
    if (connIt->get() == src) {
      mServerConnections.erase(connIt);
      break;
    } else if ((*connIt)->address() == src->address() &&
               (*connIt)->port() == src->port()) {
      // TODO is there need for this check?
      mServerConnections.erase(connIt);
      break;
    }
  }
}

bool TincServer::sendTincMessage(void *msg, al::Socket *dst,
                                 al::ValueSource *src) {
  bool ret = true;

  if (!dst) {
    // Send to all connections, except src

    std::unique_lock<std::mutex> lk(mConnectionsLock);
    for (const auto &connection : mServerConnections) {
      if (!src || (connection->address() != src->ipAddr ||
                   connection->port() != src->port)) {
        if (verbose()) {
          std::cout << "Server sending message to " << connection->address()
                    << ":" << connection->port() << std::endl;
        }
        if (connection->opened()) {
          ret &= sendProtobufMessage(msg, connection.get());
        }
      }
    }
  } else {
    // Send to dst if it is not src
    if (!src || dst->address() != src->ipAddr || dst->port() != src->port) {
      if (verbose()) {
        std::cout << "Server sending message to " << dst->address() << ":"
                  << dst->port() << std::endl;
      }
      if (dst->opened()) {
        ret &= sendProtobufMessage(msg, dst);
      }
    }
  }

  return ret;
}
