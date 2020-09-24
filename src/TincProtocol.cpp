#include "tinc/TincClient.hpp"
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

void createParameterValueMessage(al::ParameterMeta *param,
                                 ConfigureParameter &confMessage) {
  confMessage.set_id(param->getFullAddress());

  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

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
    al::Color c = p->get();
    auto *member = val.add_valuelist();
    member->set_valuefloat(c.r);
    member = val.add_valuelist();
    member->set_valuefloat(c.g);
    member = val.add_valuelist();
    member->set_valuefloat(c.b);
    member = val.add_valuelist();
    member->set_valuefloat(c.a);
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
  } else {
  }

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);
}

ConfigureParameter
createParameterSpaceIdValuesMessage(ParameterSpaceDimension *dim) {
  ConfigureParameter confMessage;
  confMessage.set_id(dim->getFullAddress());
  confMessage.set_configurationkey(ParameterConfigureType::SPACE);
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

//// ------------------------------------------------------

void TincProtocol::sendParameters(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *p : mParameters) {
    sendRegisterParameterMessage(p, dst);
  }
}

void TincProtocol::sendParameterSpace(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto ps : mParameterSpaces) {
    sendRegisterParameterSpaceMessage(ps, dst);
  }
}

void TincProtocol::sendProcessors(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *p : mProcessors) {
    sendRegisterProcessorMessage(p, dst);
  }
}

void TincProtocol::sendDataPools(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *p : mDataPools) {
    sendRegisterDataPoolMessage(p, dst);
  }
}

void TincProtocol::sendDiskBuffers(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *db : mDiskBuffers) {
    sendRegisterDiskBufferMessage(db, dst);
  }
}

void TincProtocol::sendRegisterProcessorMessage(Processor *p, al::Socket *dst) {

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

void TincProtocol::sendConfigureProcessorMessage(Processor *p,
                                                 al::Socket *dst) {

  if (strcmp(typeid(*p).name(), typeid(ProcessorAsync).name()) == 0) {
    p = dynamic_cast<ProcessorAsync *>(p)->processor();
  }

  for (auto config : p->configuration) {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PROCESSOR);
    ConfigureProcessor confMessage;
    confMessage.set_id(p->getId());
    confMessage.set_configurationkey(config.first);
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
    msg.set_allocated_details(details);

    sendProtobufMessage(&msg, dst);
  }
  if (dynamic_cast<ComputationChain *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ComputationChain *>(p)->processors()) {
      sendConfigureProcessorMessage(childProcessor, dst);
    }
  }
}

void TincProtocol::sendRegisterDataPoolMessage(DataPool *p, al::Socket *dst) {
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

  sendProtobufMessage(&msg, dst);
  sendRegisterParameterSpaceMessage(&p->getParameterSpace(), dst);
}

void TincProtocol::sendConfigureDataPoolMessage(DataPool *p, al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(CONFIGURE);
  msg.set_objecttype(DATA_POOL);
  ConfigureDataPool confMessage;
  confMessage.set_id(p->getId());
  confMessage.set_configurationkey(DataPoolConfigureType::SLICE_CACHE_DIR);
  google::protobuf::Any *configValue = confMessage.configurationvalue().New();
  ParameterValue val;
  val.set_valuestring(p->getCacheDirectory());
  configValue->PackFrom(val);
  confMessage.set_allocated_configurationvalue(configValue);
  auto details = msg.details().New();
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendRegisterDiskBufferMessage(AbstractDiskBuffer *p,
                                                 al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(DISK_BUFFER);
  RegisterDiskBuffer details;
  details.set_id(p->getId());

  DiskBufferType type = DiskBufferType::BINARY;

  if (strcmp(typeid(p).name(), typeid(NetCDFDiskBufferDouble).name()) == 0) {
    type = DiskBufferType::NETCDF;
  } else if (strcmp(typeid(p).name(), typeid(ImageDiskBuffer).name()) == 0) {
    type = DiskBufferType::IMAGE;
  } else if (strcmp(typeid(p).name(), typeid(JsonDiskBuffer).name()) == 0) {
    type = DiskBufferType::JSON;
  }
  details.set_type(type);
  details.set_basefilename(p->getBaseFileName());
  // TODO prepend node's root directory
  std::string path = al::File::currentPath() + p->getPath();
  details.set_path(path);

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);

  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendRegisterParameterSpaceMessage(ParameterSpace *p,
                                                     al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(PARAMETER_SPACE);
  RegisterParameterSpace details;
  details.set_id(p->getId());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);

  sendProtobufMessage(&msg, dst);
  for (auto dim : p->dimensions) {
    sendRegisterParameterSpaceDimensionMessage(dim.get(), dst);
  }
}

void TincProtocol::sendRegisterParameterSpaceDimensionMessage(
    ParameterSpaceDimension *dim, al::Socket *dst) {

  sendRegisterParameterMessage(&dim->parameter(), dst);
  sendParameterSpaceMessage(dim, dst);
}

void TincProtocol::sendParameterSpaceMessage(ParameterSpaceDimension *dim,
                                             al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(CONFIGURE);
  msg.set_objecttype(PARAMETER);
  ConfigureParameter confMessage = createParameterSpaceIdValuesMessage(dim);

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(confMessage);
  msg.set_allocated_details(detailsAny);

  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendParameterFloatDetails(al::Parameter *param,
                                             al::Socket *dst) {
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);

    sendProtobufMessage(&msg, dst);
  }

  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::MIN);
    ParameterValue val;
    val.set_valuefloat(param->min());
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);

    sendProtobufMessage(&msg, dst);
  }
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::MAX);
    ParameterValue val;
    val.set_valuefloat(param->max());
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);

    sendProtobufMessage(&msg, dst);
  }
}

void TincProtocol::sendParameterIntDetails(al::ParameterInt *param,
                                           al::Socket *dst) {
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);

    sendProtobufMessage(&msg, dst);
  }

  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::MIN);
    ParameterValue val;
    val.set_valueint32(param->min());
    confMessage.configurationvalue().New()->PackFrom(val);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);

    sendProtobufMessage(&msg, dst);
  }
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::MAX);
    ParameterValue val;
    val.set_valueint32(param->max());
    confMessage.configurationvalue().New()->PackFrom(val);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);

    sendProtobufMessage(&msg, dst);
  }
}

void TincProtocol::sendParameterStringDetails(al::ParameterString *param,
                                              al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(CONFIGURE);
  msg.set_objecttype(PARAMETER);

  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  createParameterValueMessage(param, confMessage);
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendParameterChoiceDetails(al::ParameterChoice *param,
                                              al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(CONFIGURE);
  msg.set_objecttype(PARAMETER);

  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  createParameterValueMessage(param, confMessage);
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendParameterColorDetails(al::ParameterColor *param,
                                             al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(CONFIGURE);
  msg.set_objecttype(PARAMETER);

  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  createParameterValueMessage(param, confMessage);
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendRegisterParameterMessage(al::ParameterMeta *param,
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
                    typeid(al::ParameterString).name()) == 0) { //
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
    auto any = details.defaultvalue().New();
    auto val = any->add_valuelist();
    val->set_valuefloat(defaultValue.r);
    val = any->add_valuelist();
    val->set_valuefloat(defaultValue.g);
    val = any->add_valuelist();
    val->set_valuefloat(defaultValue.b);
    val = any->add_valuelist();
    val->set_valuefloat(defaultValue.a);
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
    details.set_datatype(PARAMETER_TRIGGER);
  } else {
  }
  auto details_any = msg.details().New();
  details_any->PackFrom(details);
  msg.set_allocated_details(details_any);

  sendProtobufMessage(&msg, dst);

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
                    typeid(al::ParameterString).name()) == 0) { //
    al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
    sendParameterStringDetails(p, dst);
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
    sendParameterChoiceDetails(p, dst);
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
    sendParameterColorDetails(p, dst);
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    al::Trigger *p = dynamic_cast<al::Trigger *>(param);
  } else {
  }

  //
}

bool TincProtocol::runConfigure(int objectType, void *any, al::Socket *src) {
  switch (objectType) {
  case PARAMETER:
    return processConfigureParameter(any, src);
  case PROCESSOR:
  //    return sendProcessors(src);
  case DISK_BUFFER:
    return processConfigureDiskBuffer(any, src);
  case DATA_POOL:
  //    return sendDataPools(src);
  case PARAMETER_SPACE:
    //    return sendParameterSpace(src);
    break;
  }
  return false;
}

bool TincProtocol::processConfigureParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (details->Is<ConfigureParameter>()) {
    ConfigureParameter conf;
    details->UnpackTo(&conf);
    auto addr = conf.id();
    for (auto *dim : mParameterSpaceDimensions) {
      if (addr == dim->getFullAddress()) {
        ParameterConfigureType command = conf.configurationkey();
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

    for (auto *ps : mParameterSpaces) {
      for (auto dim : ps->dimensions) {
        if (addr == dim->getFullAddress()) {
          ParameterConfigureType command = conf.configurationkey();
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

    for (auto *param : mParameters) {
      if (addr == param->getFullAddress()) {
        ParameterConfigureType command = conf.configurationkey();
        if (command == ParameterConfigureType::VALUE) {
          ParameterValue v;
          if (conf.configurationvalue().Is<ParameterValue>()) {
            conf.configurationvalue().UnpackTo(&v);
            if (strcmp(typeid(*param).name(),
                       typeid(al::ParameterBool).name()) == 0) {
              al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
              p->set(v.valuefloat());
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::Parameter).name()) == 0) {
              al::Parameter *p = dynamic_cast<al::Parameter *>(param);
              p->set(v.valuefloat());
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterInt).name()) == 0) {
              al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
              p->set(v.valueint32());
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterString).name()) == 0) { //
              al::ParameterString *p =
                  dynamic_cast<al::ParameterString *>(param);
              p->set(v.valuestring());
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterPose).name()) ==
                       0) { // al::ParameterPose
              al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
              assert(1 == 0); // Implement!
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterMenu).name()) ==
                       0) { // al::ParameterMenu
              al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
              assert(1 == 0); // Implement!
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterChoice).name()) ==
                       0) { // al::ParameterChoice
              al::ParameterChoice *p =
                  dynamic_cast<al::ParameterChoice *>(param);
              p->set(v.valueuint64());
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterVec3).name()) ==
                       0) { // al::ParameterVec3
              al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
              assert(1 == 0); // Implement!
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterVec4).name()) ==
                       0) { // al::ParameterVec4
              al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
              assert(1 == 0); // Implement!
            } else if (strcmp(typeid(*param).name(),
                              typeid(al::ParameterColor).name()) ==
                       0) { // al::ParameterColor
              al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
              if (v.valuelist_size() != 4) {
                std::cerr
                    << "Unexpected number of components for ParameterColor"
                    << std::endl;
                return false;
              }
              al::Color value(
                  v.valuelist(0).valuefloat(), v.valuelist(1).valuefloat(),
                  v.valuelist(2).valuefloat(), v.valuelist(3).valuefloat());
              p->set(value);

            } else if (strcmp(typeid(*param).name(),
                              typeid(al::Trigger).name()) == 0) { // Trigger
              al::Trigger *p = dynamic_cast<al::Trigger *>(param);
            } else {
            }
            return true;
          }
        }
      }
    }

  } else {
    std::cerr << "Unexpected payload in Configure Parameter command"
              << std::endl;
  }

  return false;
}

bool TincProtocol::processConfigureParameterSpace(al::Message &message,
                                                  al::Socket *src) {

  return true;
}

bool TincProtocol::processConfigureProcessor(al::Message &message,
                                             al::Socket *src) {

  return true;
}

bool TincProtocol::processConfigureDataPool(al::Message &message,
                                            al::Socket *src) {

  return true;
}

bool TincProtocol::processConfigureDiskBuffer(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (details->Is<ConfigureDiskBuffer>()) {
    ConfigureDiskBuffer conf;
    details->UnpackTo(&conf);
    auto id = conf.id();
    for (auto *db : mDiskBuffers) {
      if (id == db->getId()) {
        DiskBufferConfigureType command = conf.configurationkey();

        if (command == DiskBufferConfigureType::CURRENT_FILE) {
          if (conf.configurationvalue().Is<ParameterValue>()) {
            ParameterValue file;
            conf.configurationvalue().UnpackTo(&file);
            if (!db->updateData(file.valuestring())) {
              std::cerr << "ERROR updating buffer from file:"
                        << file.valuestring() << std::endl;
              return false;
            }
            std::cout << "loaded " << file.valuestring() << std::endl;
            return true;
          }
        }
      }
    }
  } else {
    std::cerr << "Unexpected payload in Configure Parameter command"
              << std::endl;
  }

  return false;
}

bool TincProtocol::runCommand(int objectType, void *any, al::Socket *src) {
  switch (objectType) {
  case PARAMETER:
  //          return processCommandParameter(any, src);
  case PROCESSOR:
  //    return sendProcessors(src);
  case DISK_BUFFER:
    //          return processConfigureDiskBuffer(any, src);
    break;
  case DATA_POOL:
    return processCommandDataPool(any, src);
  case PARAMETER_SPACE:
    //    return sendParameterSpace(src);
    break;
  }
  return false;
}

bool TincProtocol::processCommandDataPool(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << "Error: Invalid payload for Command" << std::endl;
    return false;
  }
  Command command;
  details->UnpackTo(&command);
  uint32_t commandNumber = command.message_id();
  if (command.details().Is<DataPoolCommandSlice>()) {
    DataPoolCommandSlice commandSlice;
    command.details().UnpackTo(&commandSlice);
    auto datapoolId = command.id().id();
    auto field = commandSlice.field();
    std::vector<std::string> dims;
    dims.reserve(commandSlice.dimension_size());
    for (size_t i = 0; i < (size_t)commandSlice.dimension_size(); i++) {
      dims.push_back(commandSlice.dimension(i));
    }
    for (auto dp : mDataPools) {
      if (dp->getId() == datapoolId) {
        auto sliceName = dp->createDataSlice(field, dims);
        // Send slice name
        std::cout << commandNumber << "::::: " << sliceName << std::endl;
        TincMessage msg;
        msg.set_messagetype(MessageType::COMMAND_REPLY);
        msg.set_objecttype(ObjectType::DATA_POOL);
        auto *msgDetails = msg.details().New();

        Command command;
        command.set_message_id(commandNumber);
        auto commandId = command.id();
        commandId.set_id(datapoolId);

        auto *commandDetails = command.details().New();
        DataPoolCommandSliceReply reply;
        reply.set_filename(sliceName);

        commandDetails->PackFrom(reply);
        command.set_allocated_details(commandDetails);

        msgDetails->PackFrom(command);
        msg.set_allocated_details(msgDetails);

        sendProtobufMessage(&msg, src);
        return true;
      }
    }
  }
  return false;
}

void TincProtocol::requestParameters(al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REQUEST);
  msg.set_objecttype(ObjectType::PARAMETER);
  google::protobuf::Any *details = msg.details().New();
  ObjectId confMessage;
  confMessage.set_id(""); // Requests all

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  sendTincMessage(&msg);
}

void TincProtocol::requestProcessors(al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REQUEST);
  msg.set_objecttype(ObjectType::PROCESSOR);
  google::protobuf::Any *details = msg.details().New();
  ObjectId confMessage;
  confMessage.set_id(""); // Requests all

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  sendProtobufMessage(&msg, dst);
}

void TincProtocol::requestDiskBuffers(al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REQUEST);
  msg.set_objecttype(ObjectType::DISK_BUFFER);
  google::protobuf::Any *details = msg.details().New();
  ObjectId confMessage;
  confMessage.set_id(""); // Requests all

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  sendProtobufMessage(&msg, dst);
}

void TincProtocol::requestDataPools(al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REQUEST);
  msg.set_objecttype(ObjectType::DATA_POOL);
  google::protobuf::Any *details = msg.details().New();
  ObjectId confMessage;
  confMessage.set_id(""); // Requests all

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  sendProtobufMessage(&msg, dst);
}

void TincProtocol::requestParameterSpaces(al::Socket *dst) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REQUEST);
  msg.set_objecttype(ObjectType::PARAMETER_SPACE);
  google::protobuf::Any *details = msg.details().New();
  ObjectId confMessage;
  confMessage.set_id(""); // Requests all

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendParameterFloatValue(float value, std::string fullAddress,
                                           al::ValueSource *src) {
  { // Send current parameter value
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);
    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    confMessage.set_id(fullAddress);

    confMessage.set_configurationkey(ParameterConfigureType::VALUE);
    ParameterValue val;
    val.set_valuefloat(value);
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    details->PackFrom(confMessage);
    msg.set_allocated_details(details);

    sendTincMessage(&msg, src);
  }
}

void TincProtocol::sendParameterIntValue(int32_t value, std::string fullAddress,
                                         al::ValueSource *src) {

  { // Send current parameter value
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);
    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    confMessage.set_id(fullAddress);

    confMessage.set_configurationkey(ParameterConfigureType::VALUE);
    ParameterValue val;
    val.set_valueint32(value);
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    sendTincMessage(&msg, src);
  }
}
void TincProtocol::sendParameterUint64Value(uint64_t value,
                                            std::string fullAddress,
                                            al::ValueSource *src) {
  { // Send current parameter value
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);
    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    confMessage.set_id(fullAddress);

    confMessage.set_configurationkey(ParameterConfigureType::VALUE);
    ParameterValue val;
    val.set_valueuint64(value);
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    sendTincMessage(&msg, src);
  }
}

void TincProtocol::sendParameterStringValue(std::string value,
                                            std::string fullAddress,
                                            al::ValueSource *src) {
  { // Send current parameter value
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);
    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    confMessage.set_id(fullAddress);

    confMessage.set_configurationkey(ParameterConfigureType::VALUE);
    ParameterValue val;
    val.set_valuestring(value);
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    sendTincMessage(&msg, src);
  }
}

void TincProtocol::sendParameterColorValue(al::Color value,
                                           std::string fullAddress,
                                           al::ValueSource *src) {
  { // Send current parameter value
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);
    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    confMessage.set_id(fullAddress);

    confMessage.set_configurationkey(ParameterConfigureType::VALUE);
    ParameterValue val;
    ParameterValue *r = val.add_valuelist();
    r->set_valuefloat(value.r);
    ParameterValue *g = val.add_valuelist();
    g->set_valuefloat(value.g);
    ParameterValue *b = val.add_valuelist();
    b->set_valuefloat(value.b);
    ParameterValue *a = val.add_valuelist();
    a->set_valuefloat(value.a);
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    sendTincMessage(&msg, src);
  }
}

void TincProtocol::runRequest(int objectType, std::string objectId,
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

bool TincProtocol::sendProtobufMessage(void *message, al::Socket *dst) {

  google::protobuf::Message &msg =
      *static_cast<google::protobuf::Message *>(message);
  size_t size = msg.ByteSizeLong();
  char *buffer = (char *)malloc(size + sizeof(size_t));
  memcpy(buffer, &size, sizeof(size_t));
  if (!msg.SerializeToArray(buffer + sizeof(size_t), size)) {
    free(buffer);
    std::cerr << "Error serializing message" << std::endl;
  }
  auto bytes = dst->send(buffer, size + sizeof(size_t));
  if (bytes != size + sizeof(size_t)) {
    buffer[size + 1] = '\0';
    std::cerr << "Failed to send: " << buffer << std::endl;
    free(buffer);

#ifdef AL_WINDOWS
    int error = WSAGetLastError();
    std::cerr << "Winsock error: " << error << std::endl;
#endif
    return false;
  }
  free(buffer);
  return true;
}
