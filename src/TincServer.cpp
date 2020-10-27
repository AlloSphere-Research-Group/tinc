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

// ------------------------
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
          runRequest(objectType, objectId.id(), src);
        } else {
          std::cout << "Request command unexpected payload. Not ObjectId";
        }
        break;
      case MessageType::REMOVE:
        if (details.Is<ObjectId>()) {
          ObjectId objectId;
          details.UnpackTo(&objectId);
          std::cout << "Remove command received, but not implemented"
                    << std::endl;
          //              runRequest(objectType, objectId.id(), src);
        }
        break;
      case MessageType::REGISTER:
        std::cout << "server register" << std::endl;
        if (!runRegister(objectType, (void *)&details, src)) {
          std::cerr << "Error processing register command" << std::endl;
        }
        break;
      case MessageType::CONFIGURE:
        std::cout << "server configure" << std::endl;
        if (!runConfigure(objectType, (void *)&details, src)) {
          std::cerr << "Error processing configure command" << std::endl;
        }
        break;
      case MessageType::COMMAND:
        if (!runCommand(objectType, (void *)&details, src)) {
          std::cerr << "Error processing configure command" << std::endl;
        }
        break;
      case MessageType::PING:
        std::cout << "Ping command received, but not implemented" << std::endl;
        break;
      case MessageType::PONG:
        std::cout << "Pong command received, but not implemented" << std::endl;
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
  //      tincMsessage->

  //  //      CodedInputStream coded_input(&ais);
  //  auto command = message.getByte();
  //  if (command == REQUEST_PARAMETERS) {
  //    sendParameters(src);
  //    return true;
  //  } else if (command == REQUEST_PROCESSORS) {
  //    sendProcessors(src);
  //    return true;
  //  } else if (command == REQUEST_DISK_BUFFERS) {
  //    //            sendDiskBuffers();
  //    //    assert(0 == 1);
  //    return false;
  //  } else if (command == REQUEST_DATA_POOLS) {
  //    sendDataPools(src);
  //    return true;
  //  } else if (command == OBJECT_COMMAND) {
  //    return processObjectCommand(message, src);
  //  } else if (command == CONFIGURE_PARAMETER) {
  //    return processConfigureParameter(message, src);
  //  }
  return true;
}

// bool TincServer::shouldSendMessage(al::Socket *dst) {
//   // bool shouldSend = false;
//   // for (auto connection : mServerConnections) {
//   //   if (!src || connection->address() != src->ipAddr ||
//   //       connection->port() != src->port) {
//   //     sendProtobufMessage(msg, connection.get());
//   //   }
//   // }
//   return true;
// }

void TincServer::setVerbose(bool verbose) {
  CommandConnection::mVerbose = verbose;
  TincProtocol::mVerbose = verbose;
}

void TincServer::sendTincMessage(void *msg, al::ValueSource *src) {
  for (auto connection : mServerConnections) {
    if (!src || connection->address() != src->ipAddr ||
        connection->port() != src->port) {
      sendProtobufMessage(msg, connection.get());
    }
  }
}
