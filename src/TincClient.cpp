#include "tinc/TincClient.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"
#include "tinc/ProcessorAsyncWrapper.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;

TincClient::TincClient() {}

bool TincClient::processIncomingMessage(al::Message &message, al::Socket *src) {

  while (message.remainingBytes() > 8) {
    size_t msgSize;
    memcpy(&msgSize, message.data(), sizeof(size_t));
    if (msgSize > message.remainingBytes()) {
      break;
    }
    message.pushReadIndex(8);
    if (verbose()) {
      std::cout << "Client got " << msgSize << " of "
                << message.remainingBytes() << std::endl;
    }
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
        if (details.Is<ObjectId>()) {
          ObjectId objectId;
          details.UnpackTo(&objectId);
          std::cerr << __FUNCTION__
                    << ": Remove message received, but not implemented"
                    << std::endl;
        }
        break;
      case MessageType::REGISTER:
        if (verbose()) {
          std::cout << "client register received" << std::endl;
        }
        if (!readRegisterMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Register message"
                    << std::endl;
        }
        break;
      case MessageType::CONFIGURE:
        if (verbose()) {
          std::cout << "client configure received" << std::endl;
        }
        if (!readConfigureMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Configure message"
                    << std::endl;
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
        std::cerr << __FUNCTION__
                  << ": Pong message received, but not implemented"
                  << std::endl;
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
      default:
        std::cerr << __FUNCTION__ << ": Invalid message type" << std::endl;
        break;
      }
    } else {
      std::cerr << __FUNCTION__ << ": Error parsing message" << std::endl;
    }

    message.pushReadIndex(msgSize);
  }

  if (verbose()) {
    std::cout << "Message buffer : " << message.remainingBytes() << std::endl;
  }

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

bool TincClient::sendTincMessage(void *msg, al::Socket *dst,
                                 al::ValueSource *src) {
  if (!dst) {
    if (!src || mSocket.address() != src->ipAddr ||
        mSocket.port() != src->port) {
      if (verbose()) {
        std::cout << "Client sending message to " << mSocket.address() << ":"
                  << mSocket.port() << std::endl;
      }
      return sendProtobufMessage(msg, &mSocket);
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

bool TincClient::barrier(uint32_t group, float timeoutsec) {
  uint64_t currentConsecutive = 0;
  bool noCurrentBarriers = false;

  std::cerr << __FUNCTION__ << " Enter client barrier " << std::endl;
  // First flush all existing barrier requests and unlocks
  {
    std::unique_lock<std::mutex> lk(mBarrierQueuesLock);
    for (auto unlockConsecutive : mBarrierUnlocks) {
      std::deque<uint64_t>::const_iterator found;
      while (mBarrierRequests.find(unlockConsecutive.first) !=
             mBarrierRequests.end()) {
        mBarrierRequests.erase(unlockConsecutive.first);
      }
    }
    if (mBarrierRequests.size() == 1) {
      currentConsecutive = mBarrierRequests.begin()->first;
      noCurrentBarriers = true;
    } else if (mBarrierRequests.size() == 0) {
      noCurrentBarriers = true;
    } else {
      std::cerr << __FUNCTION__ << " ERROR unexpected inconsistent state in "
                                   "barrier. Aborting barriers"
                << std::endl;
      mBarrierRequests.clear();
      mBarrierUnlocks.clear();
      return false;
    }
  }

  int timems = 0;

  std::cerr << __FUNCTION__ << " Client after flush " << std::endl;
  // If no currently active barriers, wait for incoming barrier
  if (noCurrentBarriers) {
    while ((timems < (timeoutsec * 1000) || timeoutsec == 0.0)) {
      std::deque<uint64_t>::const_iterator found;
      if (mBarrierQueuesLock.try_lock()) {
        if (mBarrierRequests.size() > 0) {
          currentConsecutive = mBarrierRequests.begin()->first;
          mBarrierQueuesLock.unlock();

          TincMessage msgAck;
          msgAck.set_messagetype(MessageType::BARRIER_ACK_LOCK);
          msgAck.set_objecttype(ObjectType::GLOBAL);
          Command lockDetails;
          lockDetails.set_message_id(currentConsecutive);

          auto *commandAckDetails = msgAck.details().New();
          commandAckDetails->PackFrom(lockDetails);
          msgAck.set_allocated_details(commandAckDetails);
          bool ret = sendProtobufMessage(&msgAck,
                                         mBarrierRequests[currentConsecutive]);
          if (!ret) {
            std::cerr << "ERROR sending unlock command to "
                      << mBarrierRequests[currentConsecutive]->address() << ":"
                      << mBarrierRequests[currentConsecutive]->port()
                      << std::endl;
          }

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
  }
  if ((timems < (timeoutsec * 1000) && timeoutsec != 0.0)) {
    // Timed out waiting for barrier request
    return false;
  }

  timems = 0;
  while ((timems < (timeoutsec * 1000) || timeoutsec == 0.0)) {
    std::deque<uint64_t>::const_iterator found;
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
