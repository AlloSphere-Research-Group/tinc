#include "tinc/TincServer.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ProcessorAsync.hpp"

#include <iostream>
#include <memory>

using namespace tinc;

class Message {
public:
  Message(uint8_t *message, size_t length) : mData(message), mSize(length) {}
  std::string getString() {
    size_t startIndex = mReadIndex;
    while (mReadIndex < mSize && mData[mReadIndex] != 0x00) {
      mReadIndex++;
    }
    std::string s;
    if (mReadIndex < mSize && mReadIndex != startIndex) {
      s.resize(mReadIndex - startIndex);
      s.assign((const char *)&mData[startIndex]);
    }
    mReadIndex++; // skip final null
    return s;
  }

  uint8_t getByte() {
    uint8_t val = mData[mReadIndex];
    mReadIndex++;
    return val;
  }

  uint32_t getUint32() {
    uint32_t val;
    memcpy(&val, &mData[mReadIndex], 4);
    mReadIndex += 4;
    return val;
  }

  std::vector<std::string> getVectorString() {
    std::vector<std::string> vs;
    uint8_t count = mData[mReadIndex];
    mReadIndex++;
    for (uint8_t i = 0; i < count; i++) {
      vs.push_back(getString());
    }
    return vs;
  }

private:
  uint8_t *mData;
  const size_t mSize;
  size_t mReadIndex{0};
};

// Message parsing and packing --- Should be moved to their own class or
// namespace eventually

void insertUint32InMessage(std::vector<uint8_t> &message, const uint32_t &v) {
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

bool TincServer::processIncomingMessage(uint8_t *message, size_t length,
                                        al::Socket *src) {
  assert(length > 0);
  if (message[0] == REQUEST_PARAMETERS) {
    sendParameters();
    return true;
  } else if (message[0] == REQUEST_PROCESSORS) {
    sendProcessors();
    return true;
  } else if (message[0] == REQUEST_DISK_BUFFERS) {
    assert(0 == 1);
    //      sendDataPools();
    return false;
  } else if (message[0] == REQUEST_DATA_POOLS) {
    sendDataPools();
    return true;
  } else if (message[0] == OBJECT_COMMAND) {
    return processObjectCommand(&message[1], length - 1, src);
    ;
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
      message.push_back(PARAMETER);
      for (auto c : name) {
        message.push_back(c);
      }
      message.push_back(0);
      for (auto c : group) {
        message.push_back(c);
      }
      message.push_back(0);

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

    sendMessage(message);
  }
}

bool TincServer::processObjectCommand(uint8_t *message, size_t length,
                                      al::Socket *src) {

  auto m = Message(message, length);
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
