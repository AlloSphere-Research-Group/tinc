#include "tinc/TincServer.hpp"
#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ProcessorAsync.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;

ConfigureParameter createParameterValueMessage(al::ParameterMeta *param) {
  ConfigureParameter confMessage;
  confMessage.set_id(param->getFullAddress());

  confMessage.set_configurekey(ParameterConfigureType::VALUE);
  ParameterValue val;
  if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
    al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
    val.set_valuefloat(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0) {
    al::Parameter *p = dynamic_cast<al::Parameter *>(param);
    val.set_valuefloat(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterInt).name()) ==
             0) {
    al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
    val.set_valueint32(p->get());
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterString).name()) == 0) { // al::Parameter
    al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
    val.set_valuestring(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterPose).name()) ==
             0) { // al::ParameterPose
    al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    //    configValue->PackFrom(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterMenu).name()) ==
             0) { // al::ParameterMenu
    al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
    val.set_valueint32(p->get());
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterChoice).name()) ==
             0) { // al::ParameterChoice
    al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
    val.set_valueuint64(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec3).name()) ==
             0) { // al::ParameterVec3
    al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    //    configValue->PackFrom(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    //    configValue->PackFrom(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterColor).name()) ==
             0) { // al::ParameterColor
    al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
    //    configValue->PackFrom(p->get());
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
  } else {
  }

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);
  return confMessage;
}

ConfigureParameter
createParameterSpaceIdValuesMessage(ParameterSpaceDimension *dim) {
  ConfigureParameter confMessage;
  confMessage.set_id(dim->getFullAddress());
  confMessage.set_configurekey(ParameterConfigureType::SPACE);
  ParameterSpaceValues valuesMessage;
  for (auto &id : dim->ids()) {
    valuesMessage.add_ids(id);
  }
  for (auto &value : dim->values()) {
    valuesMessage.add_values()->set_valuefloat(value);
  }
  auto confValue = confMessage.configurationvalue().New();
  confValue->PackFrom(valuesMessage);
  confMessage.set_allocated_configurationvalue(confValue);

  return confMessage;
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

void TincServer::registerParameter(al::ParameterMeta &p) {}

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
          { // Send current parameter value
            TincMessage msg;
            msg.set_messagetype(CONFIGURE);
            msg.set_objecttype(PARAMETER);
            google::protobuf::Any *details = msg.details().New();
            details->PackFrom(createParameterValueMessage(&psd.parameter()));
            msg.set_allocated_details(details);
            size_t size = msg.ByteSizeLong();
            char *buffer = (char *)malloc(size + sizeof(size_t));
            memcpy(buffer, &size, sizeof(size_t));
            msg.SerializeToArray(buffer + sizeof(size_t), size);

            // Send message to all connections
            for (auto connection : mServerConnections) {
              if (!src || (connection->address() != src->ipAddr &&
                           connection->port() != src->port)) {
                connection->send(buffer, size);
              }
            }
            free(buffer);
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

void TincServer::runRequest(int objectType, std::string objectId,
                            al::Socket *src) {
  if (objectId.size() > 0) {
    std::cout << "Ignoring object id. Sending all requested objects."
              << std::endl;
  }
  switch (objectType) {
  case PARAMETER:
    sendParameters(src);
    break;
  case PROCESSOR:
    sendProcessors(src);
    break;
  case DISK_BUFFER:
    sendDiskBuffers(src);
    break;
  case DATA_POOL:
    sendDataPools(src);
    break;
  case PARAMETER_SPACE:
    sendParameterSpace(src);
    break;
  }
}

bool TincServer::processIncomingMessage(al::Message &message, al::Socket *src) {
  google::protobuf::io::ArrayInputStream ais(message.data(), message.size());
  google::protobuf::io::CodedInputStream codedStream(&ais);
  TincMessage *tincMessage = new TincMessage();

  while (tincMessage->ParseFromCodedStream(&codedStream) && !message.empty()) {
    auto type = tincMessage->messagetype();
    auto objectType = tincMessage->objecttype();
    auto details = tincMessage->details();

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
      runConfigure(objectType, (void *)&details, src);
      break;
    case MessageType::COMMAND:
      std::cout << "Command command received, but not implemented" << std::endl;
      break;
    case MessageType::PING:
      std::cout << "Ping command received, but not implemented" << std::endl;
      break;
    case MessageType::PONG:
      std::cout << "Pong command received, but not implemented" << std::endl;
      break;
    }
    message.pushReadIndex(codedStream.CurrentPosition());
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

void TincServer::sendParameters(al::Socket *dst) {

  //    "/registerParameter", getName(), getGroup(), getDefault(),
  //        mPrefix, min(), max()
  std::cout << __FUNCTION__ << std::endl;
  for (auto ps : mParameterSpaces) {
    for (auto psd : ps->dimensions) {
      sendRegisterParameterMessage(&psd->parameter(), dst);
    }
  }
  //  for (auto pserver : mParameterServers) {
  //    // TODO check if parameters have changed in parameter server
  //    registerParameter()
  //    for (al::ParameterMeta *param : pserver->parameters()) {
  //      auto name = param->getName();
  //      auto group = param->getName();
  //      message.reserve(2 + name.size() + 1 + group.size() + 1);
  //      message.push_back(REGISTER_PARAMETER);

  //      sendMessage(message, dst);
  //    }
  //  }

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

void TincServer::sendParameterSpace(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto ps : mParameterSpaces) {
    sendRegisterParameterSpaceMessage(ps, dst);
  }
}

void TincServer::sendProcessors(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *p : mProcessors) {
    sendRegisterProcessorMessage(p, dst);
  }
}

void TincServer::sendDataPools(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *p : mDataPools) {
    sendRegisterDataPoolMessage(p, dst);
  }
}

void TincServer::sendDiskBuffers(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *db : mDiskBuffers) {
    //    sendRegisterDiskBufferMessage(db, dst);
  }
}

void TincServer::sendRegisterProcessorMessage(Processor *p, al::Socket *dst) {

  if (strcmp(typeid(*p).name(), typeid(ProcessorAsync).name()) == 0) {
    p = dynamic_cast<ProcessorAsync *>(p)->processor();
  }
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(PROCESSOR);
  RegisterProcessor registerProcMessage;
  registerProcMessage.set_id(p->getId());
  if (strcmp(typeid(*p).name(), typeid(ScriptProcessor).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::DATASCRIPT);
  } else if (strcmp(typeid(*p).name(), typeid(ComputationChain).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::CHAIN);
  } else if (strcmp(typeid(*p).name(), typeid(CppProcessor).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::CPP);
  }

  registerProcMessage.set_inputdirectory(p->getInputDirectory());
  for (auto inFile : p->getInputFileNames()) {
    registerProcMessage.add_inputfiles(inFile);
  }
  registerProcMessage.set_outputdirectory(p->getOutputDirectory());
  for (auto outFile : p->getOutputFileNames()) {
    registerProcMessage.add_outputfiles(outFile);
  }

  sendConfigureProcessorMessage(p, dst);
  if (dynamic_cast<ComputationChain *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ComputationChain *>(p)->processors()) {
      sendRegisterProcessorMessage(childProcessor, dst);
    }
  }
}

void TincServer::sendConfigureProcessorMessage(Processor *p, al::Socket *dst) {

  if (strcmp(typeid(*p).name(), typeid(ProcessorAsync).name()) == 0) {
    p = dynamic_cast<ProcessorAsync *>(p)->processor();
  }

  for (auto config : p->configuration) {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PROCESSOR);
    ConfigureProcessor confMessage;
    confMessage.set_id(p->getId());
    confMessage.set_configurekey(config.first);
    google::protobuf::Any *configValue = confMessage.configurationvalue().New();
    ParameterValue val;
    if (config.second.type == VARIANT_DOUBLE) {
      val.set_valuedouble(config.second.valueDouble);
    } else if (config.second.type == VARIANT_INT64) {
      val.set_valueint64(config.second.valueInt64);
    } else if (config.second.type == VARIANT_STRING) {
      val.set_valuestring(config.second.valueStr);
    }
    configValue->PackFrom(val);
    auto details = msg.details().New();
    details->PackFrom(confMessage);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }
  if (dynamic_cast<ComputationChain *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ComputationChain *>(p)->processors()) {
      sendConfigureProcessorMessage(childProcessor, dst);
    }
  }
}

void TincServer::sendRegisterDataPoolMessage(DataPool *p, al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(DATA_POOL);
  RegisterDataPool details;
  details.set_id(p->getId());
  details.set_parameterspaceid(p->getParameterSpace().getId());
  details.set_cachedirectory(p->getCacheDirectory());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  size_t size = msg.ByteSizeLong();
  char *buffer = (char *)malloc(size + sizeof(size_t));
  memcpy(buffer, &size, sizeof(size_t));
  msg.SerializeToArray(buffer + sizeof(size_t), size);
  dst->send(buffer, size + sizeof(size_t));
  free(buffer);
  sendRegisterParameterSpaceMessage(&p->getParameterSpace(), dst);
}

// void TincServer::sendRegisterDiskBufferMessage(DiskBuffer *p, al::Socket
// *dst) {
//}

void TincServer::sendRegisterParameterSpaceMessage(ParameterSpace *p,
                                                   al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(PARAMETER_SPACE);
  RegisterParameterSpace details;
  details.set_id(p->getId());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  size_t size = msg.ByteSizeLong();
  char *buffer = (char *)malloc(size + sizeof(size_t));
  memcpy(buffer, &size, sizeof(size_t));
  msg.SerializeToArray(buffer + sizeof(size_t), size);
  dst->send(buffer, size + sizeof(size_t));
  free(buffer);
  for (auto dim : p->dimensions) {
    sendRegisterParameterSpaceDimensionMessage(dim.get(), dst);
  }
}

void TincServer::sendRegisterParameterSpaceDimensionMessage(
    ParameterSpaceDimension *dim, al::Socket *dst) {

  sendRegisterParameterMessage(&dim->parameter(), dst);

  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);
    ConfigureParameter confMessage = createParameterSpaceIdValuesMessage(dim);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }
}

void TincServer::sendParameterFloatDetails(al::Parameter *param,
                                           al::Socket *dst) {
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    details->PackFrom(createParameterValueMessage(param));
    msg.set_allocated_details(details);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }

  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurekey(ParameterConfigureType::MIN);
    ParameterValue val;
    val.set_valuefloat(param->min());
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurekey(ParameterConfigureType::MAX);
    ParameterValue val;
    val.set_valuefloat(param->max());
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }
}

void TincServer::sendParameterIntDetails(al::ParameterInt *param,
                                         al::Socket *dst) {
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    details->PackFrom(createParameterValueMessage(param));
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }

  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurekey(ParameterConfigureType::MIN);
    ParameterValue val;
    val.set_valueint32(param->min());
    confMessage.configurationvalue().New()->PackFrom(val);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurekey(ParameterConfigureType::MAX);
    ParameterValue val;
    val.set_valueint32(param->max());
    confMessage.configurationvalue().New()->PackFrom(val);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);
    size_t size = msg.ByteSizeLong();
    char *buffer = (char *)malloc(size + sizeof(size_t));
    memcpy(buffer, &size, sizeof(size_t));
    msg.SerializeToArray(buffer + sizeof(size_t), size);
    dst->send(buffer, size + sizeof(size_t));
    free(buffer);
  }
}

void TincServer::sendRegisterParameterMessage(al::ParameterMeta *param,
                                              al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(PARAMETER);
  RegisterParameter details;
  details.set_id(param->getName());
  details.set_group(param->getGroup());
  if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
    al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
    details.set_datatype(PARAMETER_BOOL);
    details.defaultvalue().New()->set_valuefloat(p->getDefault());
  } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0) {
    al::Parameter *p = dynamic_cast<al::Parameter *>(param);
    details.set_datatype(PARAMETER_FLOAT);
    details.defaultvalue().New()->set_valuefloat(p->getDefault());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterInt).name()) ==
             0) {
    al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
    details.set_datatype(PARAMETER_INT32);
    details.defaultvalue().New()->set_valueint32(p->getDefault());
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterString).name()) == 0) { // al::Parameter
    al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
    details.set_datatype(PARAMETER_STRING);
    details.defaultvalue().New()->set_valuestring(p->getDefault());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterPose).name()) ==
             0) { // al::ParameterPose
    al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    details.set_datatype(PARAMETER_POSED);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterMenu).name()) ==
             0) { // al::ParameterMenu
    al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
    details.set_datatype(PARAMETER_MENU);
    details.defaultvalue().New()->set_valueint32(p->getDefault());
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterChoice).name()) ==
             0) { // al::ParameterChoice
    al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
    details.set_datatype(PARAMETER_CHOICE);
    details.defaultvalue().New()->set_valueuint32(p->getDefault());
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec3).name()) ==
             0) { // al::ParameterVec3
    al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    details.set_datatype(PARAMETER_VEC3F);
    al::Vec3f defaultValue = p->getDefault();
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    details.set_datatype(PARAMETER_VEC4F);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterColor).name()) ==
             0) { // al::ParameterColor
    al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
    details.set_datatype(PARAMETER_COLORF);
    al::Color defaultValue = p->getDefault();
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
    details.set_datatype(PARAMETER_TRIGGER);
  } else {
  }
  auto details_any = msg.details().New();
  details_any->PackFrom(details);
  msg.set_allocated_details(details_any);

  size_t size = msg.ByteSizeLong();
  char *buffer = (char *)malloc(size + sizeof(size_t));
  memcpy(buffer, &size, sizeof(size_t));
  msg.SerializeToArray(buffer + sizeof(size_t), size);
  dst->send(buffer, size + sizeof(size_t));

  // Now send current value, min and max
  if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
    al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
    sendParameterFloatDetails(p, dst);
  } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0) {
    al::Parameter *p = dynamic_cast<al::Parameter *>(param);
    sendParameterFloatDetails(p, dst);
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterInt).name()) ==
             0) {
    al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
    sendParameterIntDetails(p, dst);
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
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterChoice).name()) ==
             0) { // al::ParameterChoice
    al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
    assert(1 == 0); // Implement!
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
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
  } else {
  }

  //
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
        TincMessage msg;
        msg.set_messagetype(MessageType::COMMAND_REPLY);
        assert(0 == 1);
        // TODO implement
        //        insertUint32InMessage(message, commandNumber);
        //        insertStringInMessage(message, sliceName);
        //        src->send((const char *)message.data(), message.size());
        return true;
      }
    }
  }
  return false;
}

bool TincServer::runConfigure(int objectType, void *any, al::Socket *src) {
  switch (objectType) {
  case PARAMETER:
    return processConfigureParameter(any, src);
  case PROCESSOR:
    sendProcessors(src);
    break;
  case DISK_BUFFER:
    sendDiskBuffers(src);
    break;
  case DATA_POOL:
    sendDataPools(src);
    break;
  case PARAMETER_SPACE:
    sendParameterSpace(src);
    break;
  }
}

bool TincServer::processConfigureParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (details->Is<ConfigureParameter>()) {
    ConfigureParameter conf;
    details->UnpackTo(&conf);
    auto addr = conf.id();
    for (auto *dim : mParameterSpaceDimensions) {
      if (addr == dim->getFullAddress()) {
        ParameterConfigureType command = conf.configurekey();

        if (command == ParameterConfigureType::VALUE) {
          ParameterValue v;
          if (conf.configurationvalue().Is<ParameterValue>()) {
            conf.configurationvalue().UnpackTo(&v);
            auto value = v.valuefloat();
            al::ValueSource vs{src->address(), src->port()};
            dim->parameter().set(value, &vs);
            return true;
          }
        }
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
