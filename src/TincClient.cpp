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
        // FIXME remove after debugging
        std::cout << "client register" << std::endl;
        if (!readRegisterMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Register message"
                    << std::endl;
        }
        break;
      case MessageType::CONFIGURE:
        // FIXME remove after debugging
        std::cout << "client configure" << std::endl;
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

bool TincClient::sendTincMessage(void *msg, al::Socket *dst, bool isResponse,
                                 al::ValueSource *src) {
  if (!dst) {
    if (!src || isResponse || mSocket.address() != src->ipAddr ||
        mSocket.port() != src->port) {
      if (verbose()) {
        std::cout << "Client sending message to " << mSocket.address() << ":"
                  << mSocket.port() << std::endl;
      }
      return sendProtobufMessage(msg, &mSocket);
    }
  } else {
    if (isResponse || mSocket.address() != dst->address() ||
        mSocket.port() != dst->port()) {
      if (verbose()) {
        std::cout << "Client sending message to " << mSocket.address() << ":"
                  << mSocket.port() << std::endl;
      }
      return sendProtobufMessage(msg, &mSocket);
    }
  }

  return true;
}
