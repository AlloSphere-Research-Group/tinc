#include "tinc/TincServer.hpp"
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

TincServer::TincServer() {}

bool TincServer::processIncomingMessage(al::Message &message, al::Socket *src) {

  while (message.remainingBytes() > 8) {
    size_t msgSize;
    memcpy(&msgSize, message.data(), sizeof(size_t));
    if (msgSize > message.remainingBytes()) {
      break;
    }
    message.pushReadIndex(8);
    if (verbose()) {
      std::cout << "Server got " << msgSize << " of "
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
        std::cout << "server register" << std::endl;
        if (!readRegisterMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Register message"
                    << std::endl;
        }
        break;
      case MessageType::CONFIGURE:
        // FIXME remove after debugging
        std::cout << "server configure" << std::endl;
        if (!readConfigureMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Configure message"
                    << std::endl;
        }
        break;
      case MessageType::COMMAND:
        if (!readCommandMessage(objectType, (void *)&details, src)) {
          std::cerr << __FUNCTION__ << ": Error processing Command message"
                    << std::endl;
        }
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

void TincServer::setVerbose(bool verbose) {
  CommandConnection::mVerbose = verbose;
  TincProtocol::mVerbose = verbose;
}

bool TincServer::sendTincMessage(void *msg, al::Socket *dst, bool isResponse,
                                 al::ValueSource *src) {
  bool ret = true;

  if (!dst) {
    for (auto connection : mServerConnections) {
      if (!src || connection->address() != src->ipAddr ||
          connection->port() != src->port) {
        if (verbose()) {
          std::cout << "Server sending message to " << connection->address()
                    << ":" << connection->port() << std::endl;
        }
        ret &= sendProtobufMessage(msg, connection.get());
      }
    }
    if (isResponse) {
      std::cerr << __FUNCTION__ << ": Response requested but no socket given"
                << std::endl;
    }
  } else {
    if (!isResponse) {
      for (auto connection : mServerConnections) {
        if (connection->address() != dst->address() ||
            connection->port() != dst->port()) {
          if (verbose()) {
            std::cout << "Server sending message to " << connection->address()
                      << ":" << connection->port() << std::endl;
          }
          ret &= sendProtobufMessage(msg, connection.get());
        }
      }
    } else {
      for (auto connection : mServerConnections) {
        if (connection->address() == dst->address() &&
            connection->port() == dst->port()) {
          if (verbose()) {
            std::cout << "Server responding to " << connection->address() << ":"
                      << connection->port() << std::endl;
          }
          ret = sendProtobufMessage(msg, connection.get());
          break;
        }
      }
      if (!ret) {
        std::cerr << __FUNCTION__
                  << ": Response requested but unable to send to "
                  << dst->address() << ":" << dst->port() << std::endl;
      }
    }
  }

  return ret;
}
