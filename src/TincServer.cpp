#include "tinc/TincServer.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ProcessorAsync.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/JsonDiskBuffer.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;

// ------------------------
TincServer::TincServer() {}

void TincServer::registerParameterServer(al::ParameterServer &pserver) {
  bool registered = false;
  for (auto *p : mParameterServers) {
    if (p == &pserver) {
      registered = true;
    }
  }
  if (!registered) {
    mParameterServers.push_back(&pserver);
  } else {
    std::cout << "ParameterServer already registered.";
  }
}

void TincServer::registerProcessor(Processor &processor) {
  bool registered = false;
  for (auto *p : mProcessors) {
    if (p == &processor || p->getId() == processor.getId()) {
      registered = true;
    }
  }
  if (!registered) {
    mProcessors.push_back(&processor);
    // FIXME we should register parameters registered with processors.
    //    for (auto *param: mParameters) {

    //    }
  } else {
    std::cout << "Processor: " << processor.getId() << " already registered.";
  }
}

void TincServer::registerParameterSpace(ParameterSpace &ps) {
  bool registered = false;
  for (auto *p : mParameterSpaces) {
    if (p == &ps || p->getId() == ps.getId()) {
      registered = true;
    }
  }
  for (auto dim : ps.dimensions) {
    registerParameterSpaceDimension(*dim);
  }
  if (!registered) {
    mParameterSpaces.push_back(&ps);
    ps.onDimensionRegister = [&](ParameterSpaceDimension *changedDimension,
                                 ParameterSpace *ps) {

      for (auto socket : mServerConnections) {
        for (auto dim : ps->dimensions) {
          if (dim->getName() == changedDimension->getName()) {
            sendParameterSpaceMessage(dim.get(), socket.get());
            if (dim.get() != changedDimension) {
              registerParameterSpaceDimension(*changedDimension);
            }
          }
        }
        sendRegisterParameterSpaceMessage(ps, socket.get());
      }
    };
  } else {
    if (mVerbose) {
      std::cout << "Processor: " << ps.getId() << " already registered.";
    }
  }
}

void TincServer::registerParameter(al::ParameterMeta &pmeta) {
  mParameters.push_back(&pmeta);
  al::ParameterMeta *param = &pmeta;
  // TODO ensure parameter has not been registered (directly or through a
  // parameter space)

  if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
    al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
    p->registerChangeCallback([&, p](float value, al::ValueSource *src) {
      sendParameterFloatValue(value, p->getFullAddress(), src);
    });

  } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0) {
    al::Parameter *p = dynamic_cast<al::Parameter *>(param);
    p->registerChangeCallback([&, p](float value, al::ValueSource *src) {
      sendParameterFloatValue(value, p->getFullAddress(), src);
    });
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterInt).name()) ==
             0) {
    al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
    p->registerChangeCallback([&, p](int32_t value, al::ValueSource *src) {
      sendParameterIntValue(value, p->getFullAddress(), src);
    });
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterString).name()) == 0) { // al::Parameter
    al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterPose).name()) ==
             0) { // al::ParameterPose
    al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterMenu).name()) ==
             0) { // al::ParameterMenu
    al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterChoice).name()) ==
             0) { // al::ParameterChoice
    al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
    std::cout << p->getFullAddress() << std::endl;
    p->registerChangeCallback([&, p](uint64_t value, al::ValueSource *src) {
      sendParameterUint64Value(value, p->getFullAddress(), src);
    });
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec3).name()) ==
             0) { // al::ParameterVec3
    al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterColor).name()) ==
             0) { // al::ParameterColor
    al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
    //    al::Color defaultValue = p->getDefault();
    p->registerChangeCallback([&, p](al::Color value, al::ValueSource *src) {
      sendParameterColorValue(value, p->getFullAddress(), src);
    });
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
  } else {
  }
}

void TincServer::registerParameterSpaceDimension(ParameterSpaceDimension &psd) {
  bool registered = false;
  for (auto *dim : mParameterSpaceDimensions) {
    if (dim == &psd || dim->getFullAddress() == psd.getFullAddress()) {
      registered = true;
    }
  }
  if (!registered) {
    mParameterSpaceDimensions.push_back(&psd);
    psd.parameter().registerChangeCallback(
        [&](float value, al::ValueSource *src) {
          sendParameterFloatValue(value, psd.parameter().getFullAddress(), src);
        });

    psd.onDimensionMetadataChange = [&](
        ParameterSpaceDimension *changedDimension) {
      registerParameterSpaceDimension(*changedDimension);
      for (auto socket : mServerConnections) {
        sendRegisterParameterSpaceDimensionMessage(changedDimension,
                                                   socket.get());
      }
    };
  } else {
    if (mVerbose) {
      std::cout << "ParameterSpaceDimension: " << psd.getFullAddress()
                << " already registered.";
    }
  }
}

void TincServer::registerDiskBuffer(AbstractDiskBuffer &db) {
  bool registered = false;
  for (auto *p : mDiskBuffers) {
    if (p == &db || p->getId() == db.getId()) {
      registered = true;
    }
  }
  if (!registered) {
    mDiskBuffers.push_back(&db);
  } else {

    if (mVerbose) {
      std::cout << "DiskBuffer: " << db.getId() << " already registered.";
    }
  }
}

void TincServer::registerDataPool(DataPool &dp) {
  bool registered = false;
  for (auto *p : mDataPools) {
    if (p == &dp || p->getId() == dp.getId()) {
      registered = true;
    }
  }
  registerParameterSpace(dp.getParameterSpace());
  if (!registered) {
    mDataPools.push_back(&dp);
    dp.modified = [&]() {
      for (auto dst : mServerConnections) {
        sendConfigureDataPoolMessage(&dp, dst.get());
      }
    };
  } else {
    // FIXME replace entry in mDataPools wiht this one if it is not the same
    // instance
    if (mVerbose) {
      std::cout << "DiskBuffer: " << dp.getId() << " already registered.";
    }
  }
}

TincServer &TincServer::operator<<(al::ParameterMeta &p) {
  registerParameter(p);
  return *this;
}

TincServer &TincServer::operator<<(Processor &p) {
  registerProcessor(p);
  return *this;
}

TincServer &TincServer::operator<<(ParameterSpace &p) {
  registerParameterSpace(p);
  return *this;
}

TincServer &TincServer::operator<<(AbstractDiskBuffer &db) {
  registerDiskBuffer(db);
  return *this;
}

TincServer &TincServer::operator<<(DataPool &db) {
  registerDataPool(db);
  return *this;
}

bool TincServer::processIncomingMessage(al::Message &message, al::Socket *src) {

  while (message.remainingBytes() > 8) {
    size_t msgSize;
    memcpy(&msgSize, message.data(), sizeof(size_t));
    if (msgSize > message.remainingBytes()) {
      break;
    }
    message.pushReadIndex(8);
    std::cout << "Got " << msgSize << " of " << message.remainingBytes()
              << std::endl;
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
        std::cout << "Register command received, but not implemented"
                  << std::endl;
        break;
      case MessageType::CONFIGURE:
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

  std::cout << "message buffer : " << message.remainingBytes() << std::endl;
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

void TincServer::sendTincMessage(void *msg, al::ValueSource *src) {
  for (auto connection : mServerConnections) {
    if (!src || (connection->address() != src->ipAddr &&
                 connection->port() != src->port)) {
      sendProtobufMessage(msg, connection.get());
    }
  }
}
