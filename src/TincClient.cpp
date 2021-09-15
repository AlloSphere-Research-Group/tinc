#include "tinc/TincClient.hpp"
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

TincClient::TincClient() {
  mVersion = TINC_PROTOCOL_VERSION;
  mRevision = TINC_PROTOCOL_REVISION;
}

bool TincClient::start(uint16_t serverPort, const char *serverAddr) {
  bool ret = CommandClient::start();
  if (!ret) {
    return false;
  }
  synchronize();
  return true;
}

void TincClient::stop() {
  TincMessage tincMessage;

  tincMessage.set_messagetype(MessageType::GOODBYE);
  tincMessage.set_objecttype(ObjectType::GLOBAL);

  sendTincMessage(&tincMessage);

  CommandConnection::stop();
}

bool TincClient::processIncomingMessage(al::Message &message, al::Socket *src) {

  while (message.remainingBytes() > 8) {
    size_t msgSize;
    memcpy(&msgSize, message.data(), sizeof(size_t));
    if (msgSize > message.remainingBytes()) {
      break;
    }
    message.pushReadIndex(8);
    // if (verbose()) {
    //   std::cout << "Client got " << msgSize << " of "
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
          std::cout << "Client received Remove message" << std::endl;
        }
        if (!readRemoveMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Remove "
                    << ObjectType_Name(
                           ((TincMessage *)&tincMessage)->objecttype())
                    << " message" << std::endl;
        }
        break;
      case MessageType::REGISTER:
        if (verbose()) {
          std::cout << "Client received Register message" << std::endl;
        }
        if (!readRegisterMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Register "
                    << ObjectType_Name(
                           ((TincMessage *)&tincMessage)->objecttype())
                    << " message" << std::endl;
        }
        break;
      case MessageType::CONFIGURE:
        if (verbose()) {
          std::cout << "Client received Configure message" << std::endl;
        }
        if (!readConfigureMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Configure "
                    << ObjectType_Name(
                           ((TincMessage *)&tincMessage)->objecttype())
                    << " message" << std::endl;
        }
        break;
      case MessageType::COMMAND:
        std::cerr << __FUNCTION__
                  << ": Command message received, but not implemented"
                  << std::endl;
        break;
      case MessageType::PING:
        std::cerr << __FUNCTION__
                  << ": Ping message received, but not implemented"
                  << std::endl;
        break;
      case MessageType::PONG:
        if (details.Is<ParameterValue>()) {
          ParameterValue pongDetails;
          details.UnpackTo(&pongDetails);
          {
            std::unique_lock<std::mutex> lk(mPongLock);
            mPongsReceived[src].push_back(pongDetails.valueuint64());
          }
          //          std::cout << " PONG " << pongDetails.valueuint64() <<
          //          std::endl;
        } else {
          std::cerr << __FUNCTION__ << ": Invalid payload for PONG"
                    << std::endl;
        }
        break;
      case MessageType::BARRIER_REQUEST:
        if (details.Is<Command>()) {
          Command objectId;
          details.UnpackTo(&objectId);
          processBarrierRequest(src, objectId.message_id());
        } else {
          std::cerr << __FUNCTION__ << ": Invalid payload for BARRIER_REQUEST"
                    << std::endl;
        }
        break;
      case MessageType::BARRIER_ACK_LOCK:
        std::cerr << __FUNCTION__
                  << ": Unexpected BARRIER_ACK_LOCK message in client"
                  << std::endl;
        break;
      case MessageType::BARRIER_UNLOCK:
        if (details.Is<Command>()) {
          Command objectId;
          details.UnpackTo(&objectId);
          processBarrierUnlock(src, objectId.message_id());
        } else {
          std::cerr << __FUNCTION__ << ": Invalid payload for BARRIER_UNLOCK"
                    << std::endl;
        }
        break;
      case MessageType::STATUS:
        processStatusMessage(&tincMessage);
        break;
      case MessageType::TINC_WORKING_PATH:
        processWorkingPathMessage(&tincMessage);
        break;
      default:
        std::cerr << __FUNCTION__ << ": Invalid message type" << std::endl;
        break;
      }
    } else {
      std::cerr << __FUNCTION__ << ": Error parsing message" << std::endl;
    }

    message.pushReadIndex(msgSize);
  }

  // if (verbose()) {
  //   std::cout << "Client message buffer : " << message.remainingBytes()
  //             << std::endl;
  // }

  return true;
}

void TincClient::setVerbose(bool verbose) {
  CommandConnection::mVerbose = verbose;
  TincProtocol::mVerbose = verbose;
}

void TincClient::processBarrierRequest(al::Socket *src,
                                       uint64_t barrierConsecutive) {
  std::unique_lock<std::mutex> lk(mBarrierQueuesLock);
  mBarrierRequests[barrierConsecutive] = src;
}

void TincClient::processBarrierUnlock(al::Socket *src,
                                      uint64_t barrierConsecutive) {
  std::unique_lock<std::mutex> lk(mBarrierQueuesLock);
  mBarrierUnlocks[barrierConsecutive] = src;
}

void TincClient::processStatusMessage(void *message) {
  auto msg = static_cast<TincMessage *>(message);
  auto details = msg->details();
  if (details.Is<StatusMessage>()) {
    StatusMessage statusDetails;
    details.UnpackTo(&statusDetails);
    if (msg->objecttype() == ObjectType::GLOBAL) {
      if (statusDetails.status() == StatusTypes::AVAILABLE) {
        mServerStatus = TincProtocol::Status::STATUS_AVAILABLE;
      } else if (statusDetails.status() == StatusTypes::BUSY) {
        mServerStatus = TincProtocol::Status::STATUS_BUSY;
      } else {
        mServerStatus = TincProtocol::Status::STATUS_UNKNOWN;
      }
    } else {
      std::cerr << "ERROR: non global status messages not supported"
                << std::endl;
    }
  }
}
void TincClient::processWorkingPathMessage(void *message) {

  auto msg = static_cast<TincMessage *>(message);
  auto details = msg->details();
  //    if (msg->objecttype() == ObjectType::GLOBAL)
  if (details.Is<TincPath>()) {
    TincPath path;
    details.UnpackTo(&path);
    mWorkingPath = path.path();
  } else {
    std::cerr << "Unexpected payload in TINC_WORKING_PATH message" << std::endl;
  }
}

bool TincClient::sendTincMessage(void *msg, al::Socket *dst,
                                 al::ValueSource *src) {
  if (!dst) {
    if (!src || mSocket.address() != src->ipAddr ||
        mSocket.port() != src->port) {
      if (verbose()) {
        std::cout << "Client sending message to " << mSocket.address() << ":"
                  << mSocket.port() << std::endl;
      }
      if (mSocket.opened()) {
        return sendProtobufMessage(msg, &mSocket);
      }
    }
  } else {
    if (!src || dst->address() != src->ipAddr || dst->port() != src->port) {
      if (verbose()) {
        std::cout << "Client sending message to " << dst->address() << ":"
                  << dst->port() << std::endl;
        if (dst != &mSocket) {
          std::cout << "Unexpected socket provided to client: "
                    << dst->address() << ":" << dst->port() << std::endl;
        }
      }
      return sendProtobufMessage(msg, dst);
    }
  }

  return false;
}

void TincClient::sendMetadata() {
  TincMessage msg;
  msg.set_messagetype(MessageType::TINC_CLIENT_METADATA);
  msg.set_objecttype(ObjectType::GLOBAL);
  ClientMetaData metadata;
  std::string hostName = mSocket.hostName();
  metadata.set_clienthost(hostName);

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(metadata);
  msg.set_allocated_details(detailsAny);
  sendTincMessage(&msg);
}

bool TincClient::barrier(uint32_t group, float timeoutsec) {
  std::cerr << __FUNCTION__ << " Enter client barrier " << std::endl;
  // First flush all existing barrier requests and unlocks
  {
    std::unique_lock<std::mutex> lk(mBarrierQueuesLock);
    std::vector<uint64_t> matchedUnlocks;
    for (auto unlockConsecutive : mBarrierUnlocks) {
      while (mBarrierRequests.find(unlockConsecutive.first) !=
             mBarrierRequests.end()) {
        mBarrierRequests.erase(unlockConsecutive.first);
        matchedUnlocks.push_back(unlockConsecutive.first);
      }
    }
    for (auto &matched : matchedUnlocks) {
      mBarrierUnlocks.erase(matched);
    }
    if (mBarrierRequests.size() > 1) {
      std::cerr << __FUNCTION__
                << " ERROR unexpected inconsistent state in "
                   "barrier. Aborting and flushing barriers."
                << std::endl;
      mBarrierRequests.clear();
      mBarrierUnlocks.clear();
      return false;
    }
  }

  int timems = 0;

  // process currently pending barrier wait for incoming
  uint64_t currentConsecutive = 0;
  while ((timems < (timeoutsec * 1000) || timeoutsec == 0.0)) {
    std::deque<uint64_t>::const_iterator found;
    if (mBarrierQueuesLock.try_lock()) {
      if (mBarrierRequests.size() > 0) {
        currentConsecutive = mBarrierRequests.begin()->first;

        TincMessage msgAck;
        msgAck.set_messagetype(MessageType::BARRIER_ACK_LOCK);
        msgAck.set_objecttype(ObjectType::GLOBAL);
        Command lockDetails;
        lockDetails.set_message_id(currentConsecutive);

        auto *commandAckDetails = msgAck.details().New();
        commandAckDetails->PackFrom(lockDetails);
        msgAck.set_allocated_details(commandAckDetails);
        bool ret =
            sendProtobufMessage(&msgAck, mBarrierRequests[currentConsecutive]);
        if (!ret) {
          std::cerr << "ERROR sending unlock command to "
                    << mBarrierRequests[currentConsecutive]->address() << ":"
                    << mBarrierRequests[currentConsecutive]->port()
                    << std::endl;
        }

        mBarrierRequests.erase(mBarrierRequests.begin());
        mBarrierQueuesLock.unlock();

        std::cerr << __FUNCTION__ << " Client sent ACK_LOCK has lock "
                  << currentConsecutive << std::endl;
        break;
      }
      mBarrierQueuesLock.unlock();
    }
    std::this_thread::sleep_for(
        std::chrono::milliseconds(barrierWaitGranularTimeMs));
    timems += barrierWaitGranularTimeMs;
  }

  if ((timems > (timeoutsec * 1000) && timeoutsec != 0.0)) {
    // Timed out waiting for barrier request
    return false;
  }

  // Now wait for unlock
  timems = 0;
  while ((timems < (timeoutsec * 1000) || timeoutsec == 0.0)) {
    if (mBarrierQueuesLock.try_lock()) {
      if (mBarrierUnlocks.find(currentConsecutive) != mBarrierUnlocks.end()) {
        mBarrierRequests.erase(currentConsecutive);
        mBarrierUnlocks.erase(currentConsecutive);
        mBarrierQueuesLock.unlock();
        break;
      }
      mBarrierQueuesLock.unlock();
    }
    std::this_thread::sleep_for(
        std::chrono::milliseconds(barrierWaitGranularTimeMs));
    timems += barrierWaitGranularTimeMs;
  }

  std::cerr << __FUNCTION__ << " Exit client barrier --------" << std::endl;
  return (timems >= (timeoutsec * 1000) || timeoutsec == 0.0);
}

TincClient::Status TincClient::serverStatus() { return mServerStatus; }

bool TincClient::waitForServer(float timeoutsec) {
  {
    std::unique_lock<std::mutex> lk(mBusyCountLock);
    if (mBusyCount == 0) {
      return true;
    }
  }
  mWaitForServer = true;
  auto startTime = std::chrono::high_resolution_clock::now();
  while (mWaitForServer) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(barrierWaitGranularTimeMs));
    auto now = std::chrono::high_resolution_clock::now();
    if (timeoutsec > 0.001 &&
        std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime)
                .count() > uint32_t(timeoutsec * 1e3)) {
      return false;
    }
  }
  return true;
}

uint64_t TincClient::pingServer() {
  TincMessage msg;
  msg.set_messagetype(MessageType::PING);
  msg.set_objecttype(ObjectType::GLOBAL);

  ParameterValue details;
  auto value = mPingCounter++; // Should we do an atomic operation here?
  details.set_valueuint64(value);
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);

  if (!mRunning) {
    std::cout << ("Server not connected. Ping not sent") << std::endl;
  }
  sendTincMessage(&msg);
  return value;
}

bool TincClient::waitForPong(uint64_t pingCode, float timeoutsec,
                             al::Socket *src) {
  if (!src) {
    src = &mSocket;
  }
  std::unique_lock<std::mutex> lk(mPongLock);
  float waitTime = 0.05;
  float totalWaitTime = 0;
  while (std::find(mPongsReceived[src].begin(), mPongsReceived[src].end(),
                   pingCode) == mPongsReceived[src].end()) {
    lk.unlock();
    al::al_sleep(waitTime);
    totalWaitTime += waitTime;
    if (totalWaitTime >= timeoutsec) {
      return false;
    }
    lk.lock();
  }
  mPongsReceived[src].clear();
  return true;
}
