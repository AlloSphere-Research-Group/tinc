#include "tinc/TincServer.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ProcessorAsync.hpp"

#include <iostream>
#include <memory>

using namespace tinc;

// Message parsing and packing --- Should be moved to their own class or
// namespace eventually. Perhaps using templates to simplify?

void insertUint64InMessage(std::vector<uint8_t> &message, const uint64_t &v) {
  size_t messageLen = message.size();
  message.resize(message.size() + 8);
  memcpy(&message[messageLen], &v, 8);
}

void insertUint32InMessage(std::vector<uint8_t> &message, const uint32_t &v) {
  size_t messageLen = message.size();
  message.resize(message.size() + 4);
  memcpy(&message[messageLen], &v, 4);
}

void insertInt32InMessage(std::vector<uint8_t> &message, const int32_t &v) {
  size_t messageLen = message.size();
  message.resize(message.size() + 4);
  memcpy(&message[messageLen], &v, 4);
}

void insertFloatInMessage(std::vector<uint8_t> &message, const float &v) {
  size_t messageLen = message.size();
  message.resize(message.size() + 4);
  memcpy(&message[messageLen], &v, 4);
}

void insertStringInMessage(std::vector<uint8_t> &message,
                           const std::string &s) {
  std::string inputFile;
  if (s.size() > 0) {
    size_t messageLen = message.size();
    message.resize(message.size() + s.size() + 1);
    // c_str() is guaranteed to be null terminated
    memcpy(&message[messageLen], s.c_str(), s.size() + 1);
  } else {
    message.push_back('\0');
  }
}

void insertStringVectorInMessage(std::vector<uint8_t> &message,
                                 std::vector<std::string> sv) {
  assert(sv.size() < UINT8_MAX);
  message.push_back((uint8_t)sv.size());
  for (auto &s : sv) {
    insertStringInMessage(message, s);
  }
}

void insertProcessorDefinition(std::vector<uint8_t> &message, Processor *p) {
  message.push_back(TincServer::REGISTER_PROCESSOR);
  if (strcmp(typeid(*p).name(), typeid(ScriptProcessor).name()) == 0) {
    message.push_back(TincServer::PROCESSOR_DATASCRIPT);
  } else if (strcmp(typeid(*p).name(), typeid(ComputationChain).name()) == 0) {
    message.push_back(TincServer::PROCESSOR_CHAIN);
  } else if (strcmp(typeid(*p).name(), typeid(CppProcessor).name()) == 0) {
    message.push_back(TincServer::PROCESSOR_CPP);
  } else {
    message.push_back(TincServer::PROCESSOR_UNKNOWN);
  }

  // Data is packed in these messages, should we pad instead?
  insertStringInMessage(message, p->getId());
  insertStringInMessage(message, p->getInputDirectory());
  insertStringVectorInMessage(message, p->getInputFileNames());
  insertStringInMessage(message, p->getOutputDirectory());
  insertStringVectorInMessage(message, p->getOutputFileNames());
  insertStringInMessage(message, p->getRunningDirectory());
}

void insertDataPoolDefinition(std::vector<uint8_t> &message, DataPool *p) {
  message.push_back(TincServer::REGISTER_DATA_POOL);
  insertStringInMessage(message, p->getId());
  insertStringInMessage(message, p->getParameterSpace().getId());
  insertStringInMessage(message, p->getCacheDirectory());
}

void insertParameterSpaceDefinition(std::vector<uint8_t> &message,
                                    ParameterSpace *p) {
  message.push_back(TincServer::REGISTER_PARAMETER_SPACE);
  insertStringInMessage(message, p->getId());
}

void insertParameterSpaceDimensionDefinition(std::vector<uint8_t> &message,
                                             ParameterSpaceDimension *dim) {
  auto *param = &dim->parameter();
  message.push_back(TincServer::REGISTER_PARAMETER);
  insertStringInMessage(message, param->getName());
  insertStringInMessage(message, param->getGroup());
  if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
    al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
    message.push_back(TincServer::PARAMETER_BOOL);
    insertFloatInMessage(message, p->getDefault());
  } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0) {
    al::Parameter *p = dynamic_cast<al::Parameter *>(param);
    message.push_back(TincServer::PARAMETER_FLOAT);
    insertFloatInMessage(message, p->getDefault());
    insertFloatInMessage(message, p->min());
    insertFloatInMessage(message, p->max());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterInt).name()) ==
             0) {
    al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
    message.push_back(TincServer::PARAMETER_INT32);
    insertInt32InMessage(message, p->getDefault());
    insertInt32InMessage(message, p->min());
    insertInt32InMessage(message, p->max());
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterString).name()) == 0) { // al::Parameter
    al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
    message.push_back(TincServer::PARAMETER_STRING);
    insertStringInMessage(message, p->getDefault());
    insertStringInMessage(message, p->min());
    insertStringInMessage(message, p->max());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterPose).name()) ==
             0) { // al::ParameterPose
    al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    message.push_back(TincServer::PARAMETER_POSED);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterMenu).name()) ==
             0) { // al::ParameterMenu
    al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
    int32_t defaultValue = p->getDefault();
    insertInt32InMessage(message, defaultValue);
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterChoice).name()) ==
             0) { // al::ParameterChoice
    al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
    message.push_back(TincServer::PARAMETER_CHOICE);
    insertUint64InMessage(message, p->getDefault());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec3).name()) ==
             0) { // al::ParameterVec3
    al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    message.push_back(TincServer::PARAMETER_VEC3F);
    al::Vec3f defaultValue = p->getDefault();
    insertFloatInMessage(message, defaultValue.x);
    insertFloatInMessage(message, defaultValue.y);
    insertFloatInMessage(message, defaultValue.z);
    al::Vec3f minValue = p->min();
    insertFloatInMessage(message, minValue.x);
    insertFloatInMessage(message, minValue.y);
    insertFloatInMessage(message, minValue.z);
    al::Vec3f maxValue = p->max();
    insertFloatInMessage(message, maxValue.x);
    insertFloatInMessage(message, maxValue.y);
    insertFloatInMessage(message, maxValue.z);
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    message.push_back(TincServer::PARAMETER_VEC4F);
    al::Vec4f defaultValue = p->getDefault();
    insertFloatInMessage(message, defaultValue.x);
    insertFloatInMessage(message, defaultValue.y);
    insertFloatInMessage(message, defaultValue.z);
    insertFloatInMessage(message, defaultValue.w);
    al::Vec4f minValue = p->min();
    insertFloatInMessage(message, minValue.x);
    insertFloatInMessage(message, minValue.y);
    insertFloatInMessage(message, minValue.z);
    insertFloatInMessage(message, minValue.w);
    al::Vec4f maxValue = p->max();
    insertFloatInMessage(message, maxValue.x);
    insertFloatInMessage(message, maxValue.y);
    insertFloatInMessage(message, maxValue.z);
    insertFloatInMessage(message, maxValue.w);
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterColor).name()) ==
             0) { // al::ParameterColor
    al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
    message.push_back(TincServer::PARAMETER_COLORF);
    al::Color defaultValue = p->getDefault();
    insertFloatInMessage(message, defaultValue.r);
    insertFloatInMessage(message, defaultValue.g);
    insertFloatInMessage(message, defaultValue.b);
    insertFloatInMessage(message, defaultValue.a);
    al::Color minValue = p->min();
    insertFloatInMessage(message, minValue.r);
    insertFloatInMessage(message, minValue.g);
    insertFloatInMessage(message, minValue.b);
    insertFloatInMessage(message, minValue.a);
    al::Color maxValue = p->max();
    insertFloatInMessage(message, maxValue.r);
    insertFloatInMessage(message, maxValue.g);
    insertFloatInMessage(message, maxValue.b);
    insertFloatInMessage(message, maxValue.a);
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
    message.push_back(TincServer::PARAMETER_TRIGGER);
  } else {
  }
}

void insertVariantValue(std::vector<uint8_t> &message, VariantValue &v) {
  size_t start = message.size() + 1;
  switch (v.type) {
  case FLAG_DOUBLE:
    message.push_back(TincServer::DATA_DOUBLE);
    message.resize(start + 8);
    memcpy((void *)&message.data()[start], (void *)&v.valueDouble, 8);
    break;
  case FLAG_INT64:
    message.resize(start + 8);
    memcpy((void *)&message.data()[start], (void *)&v.valueInt, 8);
    break;
  case FLAG_STRING:
    message.resize(start + v.valueStr.size());
    memcpy((void *)&message.data()[start], (void *)v.valueStr.c_str(),
           v.valueStr.size());
    break;
  }
}

void insertProcessorConfiguration(std::vector<uint8_t> &message, Processor *p) {
  if (p->configuration.size() == 0) {
    return;
  }
  message.push_back(TincServer::CONFIGURE_PROCESSOR);
  insertStringInMessage(message, p->getId());
  assert(p->configuration.size() < UINT8_MAX);
  message.push_back((uint8_t)p->configuration.size());
  for (auto config : p->configuration) {
    insertStringInMessage(message, config.first);
    insertVariantValue(message, config.second);
  }
}

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
  } else {
    std::cout << "Processor: " << ps.getId() << " already registered.";
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
          if (mServerConnections.size() == 0) {
            return;
          }
          std::vector<uint8_t> message;
          message.push_back(CONFIGURE_PARAMETER);
          insertStringInMessage(message, psd.getFullAddress());
          message.push_back(CONFIGURE_PARAMETER_VALUE);
          insertFloatInMessage(message, value);
          for (auto connection : mServerConnections) {
            if (!src || (connection->address() != src->ipAddr &&
                         connection->port() != src->port)) {
              connection->send((char *)message.data(), message.size());
            }
          }
        });
  } else {
    std::cout << "ParameterSpaceDimension: " << psd.getFullAddress()
              << " already registered.";
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
    std::cout << "DiskBuffer: " << db.getId() << " already registered.";
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
  } else {
    std::cout << "DiskBuffer: " << dp.getId() << " already registered.";
  }
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
  auto command = message.getByte();
  if (command == REQUEST_PARAMETERS) {
    sendParameters();
    return true;
  } else if (command == REQUEST_PROCESSORS) {
    sendProcessors();
    return true;
  } else if (command == REQUEST_DISK_BUFFERS) {
    //    assert(0 == 1);
    //      sendDataPools();
    return false;
  } else if (command == REQUEST_DATA_POOLS) {
    sendDataPools();
    return true;
  } else if (command == OBJECT_COMMAND) {
    return processObjectCommand(message, src);
  } else if (command == CONFIGURE_PARAMETER) {
    return processConfigureParameter(message, src);
  }
  return false;
}

void TincServer::sendParameters() {

  //    "/registerParameter", getName(), getGroup(), getDefault(),
  //        mPrefix, min(), max()
  std::cout << "sendParameters()" << std::endl;
  std::vector<uint8_t> message;
  for (auto pserver : mParameterServers) {
    // TODO check if parameters have changed in parameter server
    for (al::ParameterMeta *param : pserver->parameters()) {
      auto name = param->getName();
      auto group = param->getName();
      message.reserve(2 + name.size() + 1 + group.size() + 1);
      message.push_back(REGISTER_PARAMETER);

      sendMessage(message);
    }
  }

  // TODO implement bundles
  //  for (auto bundleGroup : mParameterBundles) {
  //    for (auto bundle : bundleGroup.second) {
  //      for (ParameterMeta *param : bundle->parameters()) {
  //        param->sendMeta(sender, bundle->name(),
  //                        std::to_string(bundle->bundleIndex()));
  //      }
  //    }
  //  }
}

void TincServer::sendProcessors() {
  for (auto *p : mProcessors) {
    sendRegisterProcessorMessage(p);
  }
}

void TincServer::sendDataPools() {
  for (auto *p : mDataPools) {
    sendRegisterDataPoolMessage(p);
  }
}

void TincServer::sendRegisterProcessorMessage(Processor *p) {

  if (strcmp(typeid(*p).name(), typeid(ProcessorAsync).name()) == 0) {
    p = dynamic_cast<ProcessorAsync *>(p)->processor();
  }
  std::vector<uint8_t> message;
  message.reserve(1024);
  insertProcessorDefinition(message, p);
  if (message.size() > 0) {
    sendMessage(message);
  }
  sendConfigureProcessorMessage(p);
  if (dynamic_cast<ComputationChain *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ComputationChain *>(p)->processors()) {
      sendRegisterProcessorMessage(childProcessor);
    }
  }
}

void TincServer::sendConfigureProcessorMessage(Processor *p) {

  if (strcmp(typeid(*p).name(), typeid(ProcessorAsync).name()) == 0) {
    p = dynamic_cast<ProcessorAsync *>(p)->processor();
  }
  std::vector<uint8_t> message;
  message.reserve(1024);
  insertProcessorConfiguration(message, p);
  if (message.size() > 0) {
    sendMessage(message);
  }
  if (dynamic_cast<ComputationChain *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ComputationChain *>(p)->processors()) {
      sendConfigureProcessorMessage(childProcessor);
    }
  }
}

void TincServer::sendRegisterDataPoolMessage(DataPool *p) {
  std::vector<uint8_t> message;
  message.reserve(1024);
  insertDataPoolDefinition(message, p);
  if (message.size() > 0) {
    sendRegisterParameterSpaceMessage(&p->getParameterSpace());
    sendMessage(message);
  }
}

void TincServer::sendRegisterParameterSpaceMessage(ParameterSpace *p) {
  std::vector<uint8_t> message;
  message.reserve(1024);
  insertParameterSpaceDefinition(message, p);
  if (message.size() > 0) {
    for (auto dim : p->dimensions) {
      sendRegisterParameterSpaceDimensionMessage(dim.get());
    }
    sendMessage(message);
  }
}

void TincServer::sendRegisterParameterSpaceDimensionMessage(
    ParameterSpaceDimension *p) {
  std::vector<uint8_t> message;
  message.reserve(1024);
  insertParameterSpaceDimensionDefinition(message, p);
  if (message.size() > 0) {
    sendMessage(message);
  }
}

bool TincServer::processObjectCommand(al::Message &m, al::Socket *src) {
  uint32_t commandNumber = m.getUint32();
  uint8_t command = m.getByte();
  if (command == CREATE_DATA_SLICE) {
    auto datapoolId = m.getString();
    auto field = m.getString();
    auto dims = m.getVectorString();
    for (auto dp : mDataPools) {
      if (dp->getId() == datapoolId) {
        auto sliceName = dp->createDataSlice(field, dims);
        // Send slice name
        std::cout << commandNumber << "::::: " << sliceName << std::endl;
        std::vector<uint8_t> message;
        message.reserve(1024);
        message.push_back(OBJECT_COMMAND_REPLY);
        insertUint32InMessage(message, commandNumber);
        insertStringInMessage(message, sliceName);
        src->send((const char *)message.data(), message.size());
        return true;
      }
    }
  }
  return false;
}

bool TincServer::processConfigureParameter(al::Message &m, al::Socket *src) {
  auto addr = m.getString();
  for (auto *dim : mParameterSpaceDimensions) {
    if (addr == dim->getFullAddress()) {
      auto command = m.getByte();
      if (command == CONFIGURE_PARAMETER_VALUE) {
        auto value = m.get<float>();
        al::ValueSource vs{src->address(), src->port()};
        dim->parameter().set(value, &vs);
        return true;
      }
    }
  }

  return false;
}

bool TincServer::processConfigureParameterSpace(al::Message &message,
                                                al::Socket *src) {

  return true;
}

bool TincServer::processConfigureProcessor(al::Message &message,
                                           al::Socket *src) {

  return true;
}

bool TincServer::processConfigureDataPool(al::Message &message,
                                          al::Socket *src) {

  return true;
}

bool TincServer::processConfigureDiskBuffer(al::Message &message,
                                            al::Socket *src) {

  return true;
}

// void TincServer::onMessage(al::osc::Message &m) {

//  if (m.addressPattern().substr(0, sizeof("/computationChains") - 1) ==
//      "/computationChains") {
//    if (m.typeTags()[0] == 'i') {
//      int port;
//      m >> port;
//      for (auto *p : mProcessors) {
//        al::osc::Packet oscPacket;
//        std::cout << "Sending info on Processor " << p->id << " to "
//                  << m.senderAddress() << ":" << port << std::endl;
//        oscPacket.beginMessage("/registerProcessor");

//        std::string type;
//        if (strcmp(typeid(*p).name(), typeid(ScriptProcessor).name()) == 0) {
//          type = "DataScript";
//        } else if (strcmp(typeid(*p).name(), typeid(ComputationChain).name())
//        ==
//                   0) {
//          type = "ComputationChain";
//        } else if (strcmp(typeid(*p).name(), typeid(CppProcessor).name()) ==
//                   0) {
//          type = "CppProcessor";
//        }

//        oscPacket << type << p->id << "";

//        // FIXME we need to support multiple input and output files
//        std::string inputFile;
//        if (p->getInputFileNames().size() > 0) {
//          inputFile = p->getInputFileNames()[0];
//        }
//        std::string outputFile;
//        if (p->getOutputFileNames().size() > 0) {
//          outputFile = p->getOutputFileNames()[0];
//        }
//        oscPacket << p->inputDirectory() << inputFile << p->outputDirectory()
//                  << outputFile << p->runningDirectory();

//        oscPacket.endMessage();

//        al::osc::Send listenerRequest(port, m.senderAddress().c_str());
//        listenerRequest.send(oscPacket);
//        for (auto config : p->configuration) {

//          oscPacket.clear();
//          oscPacket.beginMessage("/processor/configuration");
//          oscPacket << p->id << config.first;
//          switch (config.second.type) {
//          case FLAG_DOUBLE:
//            oscPacket << config.second.valueDouble;
//            break;
//          case FLAG_INT:
//            oscPacket << (int)config.second.valueInt;
//            break;
//          case FLAG_STRING:
//            oscPacket << config.second.valueStr;
//            break;
//          }

//          oscPacket.endMessage();

//          listenerRequest.send(oscPacket);
//        }

//        if (dynamic_cast<ComputationChain *>(p)) {
//          for (auto childProcessor :
//               dynamic_cast<ComputationChain *>(p)->processors()) {

//            if (strcmp(typeid(*childProcessor).name(),
//                       typeid(ProcessorAsync).name()) == 0) {
//              childProcessor =
//                  dynamic_cast<ProcessorAsync *>(childProcessor)->processor();
//            }
//            if (strcmp(typeid(*childProcessor).name(),
//                       typeid(ScriptProcessor).name()) == 0) {
//              type = "DataScript";
//            } else if (strcmp(typeid(*childProcessor).name(),
//                              typeid(ComputationChain).name()) == 0) {
//              type = "ComputationChain";
//            } else if (strcmp(typeid(*childProcessor).name(),
//                              typeid(CppProcessor).name()) == 0) {
//              type = "CppProcessor";
//            }
//            oscPacket.clear();
//            oscPacket.beginMessage("/registerProcessor");
//            // FIXME we need to support multiple input and output files
//            std::string inputFile;
//            if (childProcessor->getInputFileNames().size() > 0) {
//              inputFile = childProcessor->getInputFileNames()[0];
//            }
//            std::string outputFile;
//            if (childProcessor->getOutputFileNames().size() > 0) {
//              outputFile = childProcessor->getOutputFileNames()[0];
//            }
//            oscPacket << type << childProcessor->id << p->id
//                      << childProcessor->inputDirectory() << inputFile
//                      << childProcessor->outputDirectory() << outputFile
//                      << childProcessor->runningDirectory();

//            oscPacket.endMessage();

//            listenerRequest.send(oscPacket);
//            std::cout << "Sending info on Child Processor "
//                      << childProcessor->id << std::endl;
//            for (auto config : childProcessor->configuration) {

//              oscPacket.clear();
//              oscPacket.beginMessage("/processor/configuration");
//              oscPacket << childProcessor->id << config.first;
//              switch (config.second.type) {
//              case FLAG_DOUBLE:
//                oscPacket << config.second.valueDouble;
//                break;
//              case FLAG_INT:
//                oscPacket << (int)config.second.valueInt;
//                break;
//              case FLAG_STRING:
//                oscPacket << config.second.valueStr;
//                break;
//              }
//              oscPacket.endMessage();
//              listenerRequest.send(oscPacket);
//            }
//          }
//        }
//      }
//    } else {
//      std::cerr << "Unexpected syntax for /computationChains" << std::endl;
//      m.print();
//    }
//    return;
//  }

//  if (m.addressPattern().substr(0, sizeof("/parameterSpaces") - 1) ==
//      "/parameterSpaces") {
//    if (m.typeTags()[0] == 'i') {
//      int port;
//      m >> port;
//      for (auto *ps : mParameterSpaces) {
//      }
//    }
//    return;
//  }
//  if (m.addressPattern().substr(0, sizeof("/diskBuffers") - 1) ==
//      "/diskBuffers") {
//    if (m.typeTags()[0] == 'i') {
//      int port;
//      m >> port;
//      for (auto *db : mDiskBuffers) {
//        al::osc::Packet oscPacket;
//        std::cout << "Sending info on DiskBuffer " << db->id << " to "
//                  << m.senderAddress() << ":" << port << std::endl;
//        oscPacket.beginMessage("/registerProcessor");

//        std::string type = "DiskBuffer";
//        // TODO specilizations for specific types. Should use the visitor
//        // pattern

//        //              if (strcmp(typeid(*p).name(), typeid().name()) == 0) {
//        //                  type = "DataScript";
//        //              } else if (strcmp(typeid(*p).name(),
//        //              typeid(ComputationChain).name()) ==
//        //                         0) {
//        //                  type = "ComputationChain";
//        //              } else if (strcmp(typeid(*p).name(),
//        //              typeid(CppProcessor).name()) ==
//        //                         0) {
//        //                  type = "CppProcessor";
//        //              }

//        oscPacket << type << db->id << "";

//        oscPacket.endMessage();

//        //              al::osc::Send listenerRequest(port,
//        //              m.senderAddress().c_str());
//        //              listenerRequest.send(oscPacket);
//        //              for (auto config : p->configuration) {

//        //                  oscPacket.clear();
//        // oscPacket.beginMessage("/processor/configuration");
//        //                  oscPacket << p->id << config.first;
//        //                  switch (config.second.type) {
//        //                  case FLAG_DOUBLE:
//        //                      oscPacket << config.second.valueDouble;
//        //                      break;
//        //                  case FLAG_INT:
//        //                      oscPacket << (int)config.second.valueInt;
//        //                      break;
//        //                  case FLAG_STRING:
//        //                      oscPacket << config.second.valueStr;
//        //                      break;
//        //                  }

//        //                  oscPacket.endMessage();

//        //                  listenerRequest.send(oscPacket);
//        //              }
//      }
//    }
//    return;
//  }
//}
