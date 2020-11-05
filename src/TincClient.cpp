#include "tinc/TincClient.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"
#include "tinc/ProcessorAsync.hpp"

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
          std::cout << "Request message has unexpected payload. Not ObjectId";
        }
        break;
      case MessageType::REMOVE:
        if (details.Is<ObjectId>()) {
          ObjectId objectId;
          details.UnpackTo(&objectId);
          std::cout << "Remove message received, but not implemented"
                    << std::endl;
        }
        break;
      case MessageType::REGISTER:
        std::cout << "client register" << std::endl;
        if (!readRegisterMessage(objectType, (void *)&details, src)) {
          std::cerr << "Error processing register message" << std::endl;
        }
        break;
      case MessageType::CONFIGURE:
        std::cout << "client configure" << std::endl;
        if (!readConfigureMessage(objectType, (void *)&details, src)) {
          std::cerr << "Error processing configure message" << std::endl;
        }
        break;
      case MessageType::COMMAND:
        std::cout << "Command message received, but not implemented"
                  << std::endl;
        break;
      case MessageType::PING:
        std::cout << "Ping message received, but not implemented" << std::endl;
        break;
      case MessageType::PONG:
        std::cout << "Pong message received, but not implemented" << std::endl;
        break;
      default:
        std::cout << "Unused message type" << std::endl;
        break;
      }
    } else {
      std::cerr << "Error parsing message" << std::endl;
    }

    message.pushReadIndex(msgSize);
  }

  if (verbose()) {
    std::cout << "message buffer : " << message.remainingBytes() << std::endl;
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
