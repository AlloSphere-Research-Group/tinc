#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDF.hpp"
#include "tinc/ProcessorAsyncWrapper.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorGraph.hpp"
#include "tinc/TincClient.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;

// TODO namespace these functions to avoid potential clashes
TincMessage createRegisterParameterMessage(al::ParameterMeta *param) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::PARAMETER);
  RegisterParameter details;
  details.set_id(param->getName());
  details.set_group(param->getGroup());
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    details.set_datatype(PARAMETER_FLOAT);
    auto *def = details.mutable_defaultvalue();
    def->set_valuefloat(p->getDefault());
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param)) {
    details.set_datatype(PARAMETER_BOOL);
    auto *def = details.mutable_defaultvalue();
    def->set_valuefloat(p->getDefault());
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(param)) {
    details.set_datatype(PARAMETER_STRING);
    auto *def = details.mutable_defaultvalue();
    def->set_valuestring(p->getDefault());
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
    details.set_datatype(PARAMETER_INT32);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint32(p->getDefault());
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param)) {
    details.set_datatype(PARAMETER_VEC3F);
    al::Vec3f defaultValue = p->getDefault();
    auto *def = details.mutable_defaultvalue();
    auto *val = def->add_valuelist();
    val->set_valuefloat(defaultValue[0]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[1]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[2]);
  } else if (al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param)) {
    details.set_datatype(PARAMETER_VEC4F);
    al::Vec4f defaultValue = p->getDefault();
    auto *def = details.mutable_defaultvalue();
    auto *val = def->add_valuelist();
    val->set_valuefloat(defaultValue[0]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[1]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[2]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[3]);
  } else if (al::ParameterColor *p =
                 dynamic_cast<al::ParameterColor *>(param)) {
    details.set_datatype(PARAMETER_COLORF);
    al::Color defaultValue = p->getDefault();
    auto *def = details.mutable_defaultvalue();
    auto *val = def->add_valuelist();
    val->set_valuefloat(defaultValue.r);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue.g);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue.b);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue.a);
  } else if (al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param)) {
    details.set_datatype(PARAMETER_POSED);
    al::Pose defaultValue = p->getDefault();
    auto *def = details.mutable_defaultvalue();
    auto *val = def->add_valuelist();
    val->set_valuedouble(defaultValue.pos()[0]);
    val = def->add_valuelist();
    val->set_valuedouble(defaultValue.pos()[1]);
    val = def->add_valuelist();
    val->set_valuedouble(defaultValue.pos()[2]);
    val = def->add_valuelist();
    val->set_valuedouble(defaultValue.quat()[0]);
    val = def->add_valuelist();
    val->set_valuedouble(defaultValue.quat()[1]);
    val = def->add_valuelist();
    val->set_valuedouble(defaultValue.quat()[2]);
    val = def->add_valuelist();
    val->set_valuedouble(defaultValue.quat()[3]);
  } else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param)) {
    details.set_datatype(PARAMETER_MENU);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint32(p->getDefault());
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(param)) {
    details.set_datatype(PARAMETER_CHOICE);
    auto *def = details.mutable_defaultvalue();
    def->set_valueuint64(p->getDefault());
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
    details.set_datatype(PARAMETER_TRIGGER);
    auto *def = details.mutable_defaultvalue();
    def->set_valuebool(p->getDefault());
  } else {
    std::cerr << __FUNCTION__ << ": Unrecognized Parameter type" << std::endl;
  }

  auto details_any = msg.details().New();
  details_any->PackFrom(details);
  msg.set_allocated_details(details_any);
  return msg;
}

TincMessage createRegisterParameterSpaceMessage(ParameterSpace *ps) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::PARAMETER_SPACE);
  RegisterParameterSpace details;
  details.set_id(ps->getId());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  return msg;
}

// FIXME consider folding into individual createConfigXXXMessage functions to
// avoid multiple strcmp calls
void createParameterValueMessage(al::ParameterMeta *param,
                                 ConfigureParameter &confMessage) {
  confMessage.set_id(param->getFullAddress());

  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

  ParameterValue val;
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    val.set_valuefloat(p->get());
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param)) {
    val.set_valuefloat(p->get());
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(param)) {
    val.set_valuestring(p->get());
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
    val.set_valueint32(p->get());
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param)) {
    al::Vec3f v = p->get();
    auto *member = val.add_valuelist();
    member->set_valuefloat(v[0]);
    member = val.add_valuelist();
    member->set_valuefloat(v[1]);
    member = val.add_valuelist();
    member->set_valuefloat(v[2]);
  } else if (al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param)) {
    al::Vec4f v = p->get();
    auto *member = val.add_valuelist();
    member->set_valuefloat(v[0]);
    member = val.add_valuelist();
    member->set_valuefloat(v[1]);
    member = val.add_valuelist();
    member->set_valuefloat(v[2]);
    member = val.add_valuelist();
    member->set_valuefloat(v[3]);
  } else if (al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(
                 param)) { // al::ParameterColor
    al::Color c = p->get();
    auto *member = val.add_valuelist();
    member->set_valuefloat(c.r);
    member = val.add_valuelist();
    member->set_valuefloat(c.g);
    member = val.add_valuelist();
    member->set_valuefloat(c.b);
    member = val.add_valuelist();
    member->set_valuefloat(c.a);
  } else if (al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param)) {
    al::Pose pose = p->get();
    auto *member = val.add_valuelist();
    member->set_valuedouble(pose.pos()[0]);
    member = val.add_valuelist();
    member->set_valuedouble(pose.pos()[1]);
    member = val.add_valuelist();
    member->set_valuedouble(pose.pos()[2]);
    member = val.add_valuelist();
    member->set_valuedouble(pose.quat()[0]);
    member = val.add_valuelist();
    member->set_valuedouble(pose.quat()[1]);
    member = val.add_valuelist();
    member->set_valuedouble(pose.quat()[2]);
    member = val.add_valuelist();
    member->set_valuedouble(pose.quat()[3]);
  } else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param)) {
    val.set_valueint32(p->get());
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(param)) {
    val.set_valueuint64(p->get());
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
    val.set_valuebool(p->get());
  } else {
    std::cerr << __FUNCTION__ << ": Unrecognized Parameter type" << std::endl;
  }

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);
}

std::vector<TincMessage>
createConfigureParameterFloatMessage(al::Parameter *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

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

    messages.push_back(msg);
  }
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

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

    messages.push_back(msg);
  }
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterStringMessage(al::ParameterString *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);

    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterIntMessage(al::ParameterInt *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::MIN);
    ParameterValue val;
    val.set_valueint32(param->min());
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);

    messages.push_back(msg);
  }
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(param->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::MAX);
    ParameterValue val;
    val.set_valueint32(param->max());
    google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
    valueAny->PackFrom(val);
    confMessage.set_allocated_configurationvalue(valueAny);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);

    messages.push_back(msg);
  }
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterVec3Message(al::ParameterVec3 *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterVec4Message(al::ParameterVec4 *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterColorMessage(al::ParameterColor *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterPoseMessage(al::ParameterPose *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterMenuMessage(al::ParameterMenu *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createConfigureParameterChoiceMessage(al::ParameterChoice *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }

  return messages;
}

std::vector<TincMessage>
createConfigureParameterTriggerMessage(al::Trigger *param) {
  std::vector<TincMessage> messages;
  // We shouldn't send the value for a trigger as this will actually trigger
  // functions on the other end.
  //  {
  //    TincMessage msg;
  //    msg.set_messagetype(MessageType::CONFIGURE);
  //    msg.set_objecttype(ObjectType::PARAMETER);

  //    google::protobuf::Any *details = msg.details().New();
  //    ConfigureParameter confMessage;
  //    createParameterValueMessage(param, confMessage);
  //    details->PackFrom(confMessage);
  //    msg.set_allocated_details(details);
  //    messages.push_back(msg);
  //  }

  return messages;
}

std::vector<TincMessage>
createConfigureParameterMessage(al::ParameterMeta *param) {
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    return createConfigureParameterFloatMessage(p);
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param)) {
    return createConfigureParameterFloatMessage(p);
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(param)) {
    return createConfigureParameterStringMessage(p);
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
    return createConfigureParameterIntMessage(p);
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param)) {
    return createConfigureParameterVec3Message(p);
  } else if (al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param)) {
    return createConfigureParameterVec4Message(p);
  } else if (al::ParameterColor *p =
                 dynamic_cast<al::ParameterColor *>(param)) {
    return createConfigureParameterColorMessage(p);
  } else if (al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param)) {
    return createConfigureParameterPoseMessage(p);
  } else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param)) {
    return createConfigureParameterMenuMessage(p);
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(param)) {
    return createConfigureParameterChoiceMessage(p);
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
    return createConfigureParameterTriggerMessage(p);
  }

  std::cerr << __FUNCTION__ << ": Unrecognized Parameter type" << std::endl;
  return {};
}

std::vector<TincMessage>
createConfigureParameterSpaceDimensionMessage(ParameterSpaceDimension *dim) {

  std::vector<TincMessage> confMessages =
      createConfigureParameterMessage(dim->getParameterMeta());

  if (dim->size() > 0) {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PARAMETER);

    ConfigureParameter confMessage;
    confMessage.set_id(dim->getFullAddress());
    confMessage.set_configurationkey(ParameterConfigureType::SPACE);

    ParameterSpaceValues valuesMessage;
    for (auto &id : dim->getSpaceIds()) {
      valuesMessage.add_ids(id);
    }
    if (dim->getSpaceDataType() == al::DiscreteParameterValues::FLOAT) {
      for (auto &value : dim->getSpaceValues<float>()) {
        valuesMessage.add_values()->set_valuefloat(value);
      }
    } else if (dim->getSpaceDataType() == al::DiscreteParameterValues::UINT8) {
      for (auto &value : dim->getSpaceValues<uint8_t>()) {
        valuesMessage.add_values()->set_valueuint8(value);
      }
    } else if (dim->getSpaceDataType() == al::DiscreteParameterValues::INT32) {
      for (auto &value : dim->getSpaceValues<int32_t>()) {
        valuesMessage.add_values()->set_valueint32(value);
      }
    } else if (dim->getSpaceDataType() == al::DiscreteParameterValues::UINT32) {
      for (auto &value : dim->getSpaceValues<int32_t>()) {
        valuesMessage.add_values()->set_valueint32(value);
      }
    }
    // FIXME support the rest of the types
    auto confValue = confMessage.configurationvalue().New();
    confValue->PackFrom(valuesMessage);
    confMessage.set_allocated_configurationvalue(confValue);

    google::protobuf::Any *detailsAny = msg.details().New();
    detailsAny->PackFrom(confMessage);
    msg.set_allocated_details(detailsAny);
    confMessages.push_back(msg);
  }

  return confMessages;
}

TincMessage createConfigureDataPoolMessage(DataPool *dp) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::DATA_POOL);
  ConfigureDataPool confMessage;
  confMessage.set_id(dp->getId());
  confMessage.set_configurationkey(DataPoolConfigureType::SLICE_CACHE_DIR);
  google::protobuf::Any *configValue = confMessage.configurationvalue().New();
  ParameterValue val;
  val.set_valuestring(dp->getCacheDirectory());
  configValue->PackFrom(val);
  confMessage.set_allocated_configurationvalue(configValue);
  auto details = msg.details().New();
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  return msg;
}

TincMessage createConfigureParameterSpaceAdd(ParameterSpace *ps,
                                             ParameterSpaceDimension *dim) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER_SPACE);
  ConfigureParameterSpace confMessage;
  confMessage.set_id(ps->getId());
  confMessage.set_configurationkey(ParameterSpaceConfigureType::ADD_PARAMETER);
  google::protobuf::Any *configValue = confMessage.configurationvalue().New();
  ParameterValue val;
  val.set_valuestring(dim->getFullAddress());
  configValue->PackFrom(val);
  confMessage.set_allocated_configurationvalue(configValue);
  auto details = msg.details().New();
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  return msg;
}

TincMessage createConfigureParameterSpaceRemove(ParameterSpace *ps,
                                                ParameterSpaceDimension *dim) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER_SPACE);
  ConfigureParameterSpace confMessage;
  confMessage.set_id(ps->getId());
  confMessage.set_configurationkey(
      ParameterSpaceConfigureType::REMOVE_PARAMETER);
  google::protobuf::Any *configValue = confMessage.configurationvalue().New();
  ParameterValue val;
  val.set_valuestring(dim->getFullAddress());
  configValue->PackFrom(val);
  confMessage.set_allocated_configurationvalue(configValue);
  auto details = msg.details().New();
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  return msg;
}

bool processConfigureParameterValueMessage(ConfigureParameter &conf,
                                           al::ParameterMeta *param,
                                           al::Socket *src) {
  auto &confValue = conf.configurationvalue();
  if (!confValue.Is<ParameterValue>()) {
    std::cerr << __FUNCTION__ << ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }
  const ParameterConfigureType &command = conf.configurationkey();
  ParameterValue v;
  confValue.UnpackTo(&v);
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valuefloat(), src->valueSource());
    } else if (command == ParameterConfigureType::MIN) {
      p->min(v.valuefloat(), src->valueSource());
    } else if (command == ParameterConfigureType::MAX) {
      p->max(v.valuefloat(), src->valueSource());
    }
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valuefloat(), src->valueSource());
    } else if (command == ParameterConfigureType::MIN) {
      p->min(v.valuefloat(), src->valueSource());
    } else if (command == ParameterConfigureType::MAX) {
      p->max(v.valuefloat(), src->valueSource());
    }
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valuestring(), src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterString"
                << std::endl;
      return false;
    }
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valueint32(), src->valueSource());
    } else if (command == ParameterConfigureType::MIN) {
      p->min(v.valueint32(), src->valueSource());
    } else if (command == ParameterConfigureType::MAX) {
      p->max(v.valueint32(), src->valueSource());
    }
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      if (v.valuelist_size() != 3) {
        std::cerr << __FUNCTION__
                  << ": Unexpected number of components for ParameterVec3"
                  << std::endl;
        return false;
      }
      al::Vec3f value(v.valuelist(0).valuefloat(), v.valuelist(1).valuefloat(),
                      v.valuelist(2).valuefloat());
      p->set(value, src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterVec3" << std::endl;
      return false;
    }
  } else if (al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      if (v.valuelist_size() != 4) {
        std::cerr << __FUNCTION__
                  << ": Unexpected number of components for ParameterVec4"
                  << std::endl;
        return false;
      }
      al::Vec4f value(v.valuelist(0).valuefloat(), v.valuelist(1).valuefloat(),
                      v.valuelist(2).valuefloat(), v.valuelist(3).valuefloat());
      p->set(value, src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterVec4" << std::endl;
      return false;
    }
  } else if (al::ParameterColor *p =
                 dynamic_cast<al::ParameterColor *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      if (v.valuelist_size() != 4) {
        std::cerr << __FUNCTION__
                  << ": Unexpected number of components for ParameterColor"
                  << std::endl;
        return false;
      }
      al::Color value(v.valuelist(0).valuefloat(), v.valuelist(1).valuefloat(),
                      v.valuelist(2).valuefloat(), v.valuelist(3).valuefloat());
      p->set(value, src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterColor" << std::endl;
      return false;
    }
  } else if (al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      if (v.valuelist_size() != 7) {
        std::cerr << __FUNCTION__
                  << ": Unexpected number of components for ParameterPose"
                  << std::endl;
        return false;
      }
      al::Pose value(
          {v.valuelist(0).valuedouble(), v.valuelist(1).valuedouble(),
           v.valuelist(2).valuedouble()},
          {v.valuelist(3).valuedouble(), v.valuelist(4).valuedouble(),
           v.valuelist(5).valuedouble(), v.valuelist(6).valuedouble()});
      p->set(value, src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterPose" << std::endl;
      return false;
    }
  } else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valueint32(), src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterMenu" << std::endl;
      return false;
    }
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valueuint64(), src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterChoice"
                << std::endl;
      return false;
    }
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
    if (command == ParameterConfigureType::VALUE) {
      p->set(v.valuebool(), src->valueSource());
    } else {
      std::cerr << __FUNCTION__
                << ": Unexpected min/max value for ParameterTrigger"
                << std::endl;
      return false;
    }
  } else {
    std::cerr << __FUNCTION__ << ": Unrecognized Parameter type" << std::endl;
    return false;
  }

  return true;
}

bool processConfigureParameterMessage(ConfigureParameter &conf,
                                      ParameterSpaceDimension *dim,
                                      al::Socket *src) {

  ParameterConfigureType command = conf.configurationkey();

  if (command == ParameterConfigureType::SPACE) {
    ParameterSpaceValues sv;
    if (conf.configurationvalue().Is<ParameterSpaceValues>()) {
      conf.configurationvalue().UnpackTo(&sv);
      auto values = sv.values();
      auto idsIt = sv.ids().begin();
      if (sv.ids().size() != 0 && sv.ids().size() != values.size()) {
        std::cerr << "ids size mismatch. ignoring" << std::endl;
        idsIt = sv.ids().end();
      }
      dim->clear(src);
      if (dim->getSpaceDataType() == al::DiscreteParameterValues::FLOAT) {
        std::vector<float> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valuefloat());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        if (newIds.size() > 0 || newValues.size() > 0) {
          dim->setSpaceValues(newValues, "", src);
          dim->setSpaceIds(newIds, src);
          dim->conformSpace();
        } else {
          std::cout << "Warning: got empty parameter space values" << std::endl;
        }
      } else if (dim->getSpaceDataType() ==
                 al::DiscreteParameterValues::UINT8) {
        std::vector<uint8_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueuint8());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        if (newIds.size() > 0 || newValues.size() > 0) {
          dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
          dim->setSpaceIds(newIds, src);
          dim->conformSpace();
        } else {
          std::cout << "Warning: got empty parameter space values" << std::endl;
        }
      } else if (dim->getSpaceDataType() ==
                 al::DiscreteParameterValues::INT32) {
        std::vector<int32_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueint32());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        if (newIds.size() > 0 || newValues.size() > 0) {
          dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
          dim->setSpaceIds(newIds, src);
          dim->conformSpace();
        } else {
          std::cout << "Warning: got empty parameter space values" << std::endl;
        }
      } else if (dim->getSpaceDataType() ==
                 al::DiscreteParameterValues::UINT32) {
        std::vector<uint32_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueuint32());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        if (newIds.size() > 0 || newValues.size() > 0) {
          dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
          dim->setSpaceIds(newIds, src);
          dim->conformSpace();
        } else {
          std::cout << "Warning: got empty parameter space values" << std::endl;
        }
      }
      // FIXME add all types
    } else {
      return false;
    }
    // TODO ensure correct forwarding to connections
    return true;
  } else if (command == ParameterConfigureType::SPACE_TYPE) {

    if (conf.configurationvalue().Is<ParameterValue>()) {
      ParameterValue v;
      conf.configurationvalue().UnpackTo(&v);
      dim->setSpaceRepresentationType(
          (ParameterSpaceDimension::RepresentationType)v.valueint32(), src);
      // TODO ensure correct forwarding to connections
      return true;

    } else {
      return false;
    }
  }
  al::ParameterMeta *param = dim->getParameterMeta();
  return processConfigureParameterValueMessage(conf, param, src);
}

//// ------------------------------------------------------
void TincProtocol::registerParameter(al::ParameterMeta &pmeta,
                                     al::Socket *src) {
  bool registered = false;
  for (auto dim : mParameterSpaceDimensions) {
    if (dim->getName() == pmeta.getName() &&
        dim->getGroup() == pmeta.getGroup()) {
      registered = true;
      if (mVerbose) {
        std::cout << __FUNCTION__ << ": Parameter " << pmeta.getName()
                  << " (Group: " << pmeta.getGroup() << ") already registered."
                  << std::endl;
      }
      if (&pmeta != dim->getParameterMeta()) {
        // FIXME this will create a new parameter with same id/group
        std::cerr
            << __FUNCTION__
            << ": Parameter is already registered but pointer doesn't match."
            << std::endl;
      }
      break;
    }
  }
  if (!registered) {
    mLocalPSDs.emplace_back(
        std::make_unique<ParameterSpaceDimension>(&pmeta, false));
    registerParameterSpaceDimension(*mLocalPSDs.back(), src);
  }
}

void TincProtocol::registerParameterSpaceDimension(ParameterSpaceDimension &psd,
                                                   al::Socket *src) {
  ParameterSpaceDimension *registered = nullptr;
  for (auto *dim : mParameterSpaceDimensions) {
    if (dim == &psd || dim->getFullAddress() == psd.getFullAddress()) {
      registered = dim;
      if (mVerbose) {
        std::cout << __FUNCTION__ << ": ParameterSpaceDimension "
                  << psd.getFullAddress() << " already registered."
                  << std::endl;
      }
      break;
    }
  }
  for (auto *ps : mParameterSpaces) {
    for (auto dim : ps->getDimensions()) {
      if (dim.get() == &psd || dim->getFullAddress() == psd.getFullAddress()) {
        registered = dim.get();
        if (mVerbose) {
          std::cout << __FUNCTION__ << ": ParameterSpaceDimension "
                    << psd.getFullAddress() << " already registered."
                    << std::endl;
        }
        break;
      }
    }
  }
  if (!registered) {
    mParameterSpaceDimensions.push_back(&psd);
    connectParameterCallbacks(*psd.getParameterMeta());
    connectDimensionCallbacks(psd);

    // Broadcast registered ParameterSpaceDimension
    sendRegisterMessage(&psd, nullptr, src);
    sendConfigureMessage(&psd, nullptr, src);
  } else {
    connectParameterCallbacks(*registered->getParameterMeta());
    connectDimensionCallbacks(*registered);
    sendRegisterMessage(registered, nullptr, src);
    sendConfigureMessage(registered, nullptr, src);
  }
}

void TincProtocol::registerParameterSpace(ParameterSpace &ps, al::Socket *src) {
  bool registered = false;
  for (auto *p : mParameterSpaces) {
    if (p == &ps || p->getId() == ps.getId()) {
      registered = true;
      if (mVerbose) {
        std::cout << __FUNCTION__ << ": Processor " << ps.getId()
                  << " already registered." << std::endl;
      }
      break;
    }
  }
  if (!registered) {
    mParameterSpaces.push_back(&ps);

    // FIXME re-check callback function. something doesn't look right
    ps.onDimensionRegister = [this](ParameterSpaceDimension *changedDimension,
                                    ParameterSpace *ps, al::Socket *src) {
      for (auto dim : ps->getDimensions()) {
        // check if dimension already exists
        if (dim->getName() == changedDimension->getName()) {
          // FIXME dimension pointer gets stored in 2 places
          // review cases where this redundancy is needed
          // (connecting callbacks, etc)

          // connects callbacks, sends register + configure messages
          registerParameterSpaceDimension(*changedDimension, src);
          sendConfigureParameterSpaceAddDimension(ps, dim.get(), nullptr, src);
          sendConfigureMessage(ps, nullptr, src);
          break;
        }
      }
    };

    // register PSDs attached to the ParameterSpace
    for (auto dim : ps.getDimensions()) {
      registerParameterSpaceDimension(*dim, src);
    }

    // Broadcast registered ParameterSpace
    // FIXME if the parameter was not registered above, this will send
    // register/config message for the parameter twice here
    sendRegisterMessage(&ps, nullptr, src);
    sendConfigureMessage(&ps, nullptr, src);
  }
}

void TincProtocol::registerProcessor(Processor &processor, al::Socket *src) {
  bool registered = false;
  for (auto *p : mProcessors) {
    if (p == &processor || p->getId() == processor.getId()) {
      registered = true;
      if (mVerbose) {
        std::cout << __FUNCTION__ << ": Processor " << processor.getId()
                  << " already registered." << std::endl;
      }
      break;
    }
  }
  if (!registered) {
    mProcessors.push_back(&processor);
    // FIXME we should register parameters registered with processors.
    //    for (auto *param: processor.parameters()) {
    //    }

    // Broadcast registered processor
    sendRegisterMessage(&processor, nullptr, src);
    sendConfigureMessage(&processor, nullptr, src);
  }
}

void TincProtocol::registerDiskBuffer(DiskBufferAbstract &db, al::Socket *src) {
  bool registered = false;
  for (auto *p : mDiskBuffers) {
    if (p == &db || p->getId() == db.getId()) {
      registered = true;
      if (mVerbose) {
        std::cout << __FUNCTION__ << ": DiskBuffer " << db.getId()
                  << " already registered." << std::endl;
      }
      break;
    }
  }
  if (!registered) {
    mDiskBuffers.push_back(&db);

    // Broadcast registered DiskBuffer
    sendRegisterMessage(&db, nullptr, src);
    sendConfigureMessage(&db, nullptr, src);
  } else {
    // TODO apply data from new dimension
  }
}

void TincProtocol::registerDataPool(DataPool &dp, al::Socket *src) {
  bool registered = false;
  for (auto *p : mDataPools) {
    if (p == &dp || p->getId() == dp.getId()) {
      registered = true;
      if (mVerbose) {
        // FIXME replace entry in mDataPools wiht this one if it is not the same
        // instance
        std::cout << __FUNCTION__ << ": DataPool " << dp.getId()
                  << " already registered." << std::endl;
      }
      break;
    }
  }
  if (!registered) {
    mDataPools.push_back(&dp);
    // FIXME check register order datapool -> ps
    registerParameterSpace(dp.getParameterSpace(), src);

    // FIXME add input source to avoid repropagating
    dp.modified = [&]() {
      auto msg = createConfigureDataPoolMessage(&dp);
      sendTincMessage(&msg);
    };

    // Broadcast registered DataPool
    sendRegisterMessage(&dp, nullptr, src);
    sendConfigureMessage(&dp, nullptr, src);
  }
}

TincProtocol &TincProtocol::operator<<(al::ParameterMeta &p) {
  registerParameter(p);
  return *this;
}

TincProtocol &TincProtocol::operator<<(ParameterSpace &ps) {
  registerParameterSpace(ps);
  return *this;
}

TincProtocol &TincProtocol::operator<<(ParameterSpaceDimension &psd) {
  registerParameterSpaceDimension(psd);
  return *this;
}

TincProtocol &TincProtocol::operator<<(Processor &p) {
  registerProcessor(p);
  return *this;
}

TincProtocol &TincProtocol::operator<<(DiskBufferAbstract &db) {
  registerDiskBuffer(db);
  return *this;
}

TincProtocol &TincProtocol::operator<<(DataPool &dp) {
  registerDataPool(dp);
  return *this;
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

ParameterSpaceDimension *TincProtocol::getDimension(std::string name,
                                                    std::string group) {
  for (auto dim : mParameterSpaceDimensions) {
    if (dim->getName() == name && dim->getGroup() == group) {
      return dim;
    } else if (group == "" && dim->getFullAddress() == name) {
      return dim;
    }
  }
  for (auto *ps : mParameterSpaces) {
    auto dim = ps->getDimension(name, group);
    if (dim) {
      return dim.get();
    }
  }
  return nullptr;
}

al::ParameterMeta *TincProtocol::getParameter(std::string name,
                                              std::string group) {
  for (auto *dim : mParameterSpaceDimensions) {
    if (dim->getName() == name && dim->getGroup() == group) {
      return dim->getParameterMeta();
    } else if (group == "" && dim->getName() == name) {
      return dim->getParameterMeta();
    }
  }
  return nullptr;
}

void TincProtocol::markBusy() {
  std::unique_lock<std::mutex> lk(mBusyCountLock);
  assert(mBusyCount < UINT32_MAX);
  mBusyCount++;
}

void TincProtocol::markAvailable() {
  std::unique_lock<std::mutex> lk(mBusyCountLock);
  if (mBusyCount == 0) {
    std::cerr << __FUNCTION__ << "ERROR: Can't mark as available. Not busy."
              << std::endl;
    return;
  }
  mBusyCount--;
}

TincProtocol::Status TincProtocol::getStatus() {
  std::unique_lock<std::mutex> lk(mBusyCountLock);
  if (mBusyCount > 0) {
    return STATUS_BUSY;
  } else {
    return STATUS_AVAILABLE;
  }
}

void TincProtocol::connectParameterCallbacks(al::ParameterMeta &pmeta) {
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(&pmeta)) {
    p->registerChangeCallback([&, p](float value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(&pmeta)) {
    p->registerChangeCallback([&, p](float value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(&pmeta)) {
    p->registerChangeCallback([&, p](std::string value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(&pmeta)) {
    p->registerChangeCallback([&, p](int32_t value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(&pmeta)) {
    p->registerChangeCallback([&, p](al::Vec3f value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(&pmeta)) {
    p->registerChangeCallback([&, p](al::Vec4f value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterColor *p =
                 dynamic_cast<al::ParameterColor *>(&pmeta)) {
    p->registerChangeCallback([&, p](al::Color value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(&pmeta)) {
    p->registerChangeCallback([&, p](al::Pose value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(&pmeta)) {
    p->registerChangeCallback([&, p](int32_t value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(&pmeta)) {
    p->registerChangeCallback([&, p](uint64_t value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
      std::vector<TincMessage> confMessages =
          createConfigureParameterMessage(&pmeta);
      for (auto &msg : confMessages) {
        sendTincMessage(&msg, nullptr, src);
      }
    });
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(&pmeta)) {
    p->registerChangeCallback([&, p](bool value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->setNoCalls(
        false); // Default value needs to be set manually in some cases.
    //    p->registerMetaChangeCallback([&, p](al::ValueSource *src) {
    //        std::vector<TincMessage> confMessages =
    //            createConfigureParameterMessage(&pmeta);
    //        for (auto &msg: confMessages) {
    //            sendTincMessage(&msg, nullptr, src);
    //        }
    //    });
  }
}

void TincProtocol::connectDimensionCallbacks(ParameterSpaceDimension &psd) {
  psd.onDimensionMetadataChange = [&](ParameterSpaceDimension *changedDimension,
                                      al::Socket *src) {
    // FIXME register necessary here?
    registerParameterSpaceDimension(*changedDimension, src);

    TincMessage msg =
        createRegisterParameterMessage(changedDimension->getParameterMeta());
    if (src) {
      sendTincMessage(&msg, nullptr, src->valueSource());
    } else {
      sendTincMessage(&msg);
    }

    auto msgs = createConfigureParameterSpaceDimensionMessage(changedDimension);
    for (auto &msg : msgs) {
      if (src) {
        sendTincMessage(&msg, nullptr, src->valueSource());
      } else {
        sendTincMessage(&msg);
      }
    }
  };
}

void TincProtocol::readRequestMessage(int objectType, std::string objectId,
                                      al::Socket *src) {
  if (objectId.size() > 0) {
    std::cout << __FUNCTION__
              << ": Ignoring object id. Sending all requested objects."
              << std::endl;
  }
  switch (objectType) {
  case ObjectType::PARAMETER:
    processRequestParameters(src);
    break;
  case ObjectType::PROCESSOR:
    processRequestProcessors(src);
    break;
  case ObjectType::DISK_BUFFER:
    processRequestDiskBuffers(src);
    break;
  case ObjectType::DATA_POOL:
    processRequestDataPools(src);
    break;
  case ObjectType::PARAMETER_SPACE:
    processRequestParameterSpaces(src);
    break;
  default:
    std::cerr << __FUNCTION__ << ": Invalid ObjectType" << std::endl;
    break;
  }
}

void TincProtocol::processRequestParameters(al::Socket *dst) {
  for (auto *dim : mParameterSpaceDimensions) {
    sendRegisterMessage(dim, dst);
    sendConfigureMessage(dim, dst);
  }
}

void TincProtocol::processRequestParameterSpaces(al::Socket *dst) {
  for (auto ps : mParameterSpaces) {
    sendRegisterMessage(ps, dst);
    sendConfigureMessage(ps, dst);
  }
}

void TincProtocol::processRequestProcessors(al::Socket *dst) {
  for (auto *p : mProcessors) {
    sendRegisterMessage(p, dst);
    sendConfigureMessage(p, dst);
  }
}

void TincProtocol::processRequestDiskBuffers(al::Socket *dst) {
  for (auto *db : mDiskBuffers) {
    sendRegisterMessage(db, dst);
    sendConfigureMessage(db, dst);
  }
}

void TincProtocol::processRequestDataPools(al::Socket *dst) {
  for (auto *dp : mDataPools) {
    sendRegisterMessage(dp, dst);
    sendConfigureMessage(dp, dst);
  }
}

bool TincProtocol::readRegisterMessage(int objectType, void *any,
                                       al::Socket *src) {
  switch (objectType) {
  case ObjectType::PARAMETER:
    return processRegisterParameter(any, src);
  case ObjectType::PROCESSOR:
    return processRegisterProcessor(any, src);
    break;
  case ObjectType::DISK_BUFFER:
    return processRegisterDiskBuffer(any, src);
  case ObjectType::DATA_POOL:
    return processRegisterDataPool(any, src);
    break;
  case ObjectType::PARAMETER_SPACE:
    return processRegisterParameterSpace(any, src);
    break;
  default:
    std::cerr << __FUNCTION__ << ": Invalid ObjectType" << std::endl;
    break;
  }
  return false;
}

bool TincProtocol::processRegisterParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterParameter>()) {
    std::cerr << __FUNCTION__ << ": Register message contains invalid payload"
              << std::endl;
    return false;
  }

  RegisterParameter command;
  details->UnpackTo(&command);
  auto id = command.id();
  auto group = command.group();
  auto def = command.defaultvalue();
  auto datatype = command.datatype();

  for (auto dim : mParameterSpaceDimensions) {
    if (dim->getName() == id && dim->getGroup() == group) {
      if (mVerbose) {
        // FIXME apply configuration (min, max, default) if found
        std::cout << __FUNCTION__ << ": Parameter " << id
                  << " (Group: " << group << ") already registered."
                  << std::endl;
      }
      return true;
    }
  }

  al::ParameterMeta *param = nullptr;
  switch (datatype) {
  case ParameterDataType::PARAMETER_FLOAT:
    param = new al::Parameter(id, group, def.valuefloat());
    break;
  case ParameterDataType::PARAMETER_BOOL:
    param = new al::ParameterBool(id, group, def.valuefloat());
    break;
  case ParameterDataType::PARAMETER_STRING:
    param = new al::ParameterString(id, group, def.valuestring());
    break;
  case ParameterDataType::PARAMETER_INT32:
    param = new al::ParameterInt(id, group, def.valueint32());
    break;
  case ParameterDataType::PARAMETER_VEC3F: {
    al::Vec3f value(def.valuelist(0).valuefloat(),
                    def.valuelist(1).valuefloat(),
                    def.valuelist(2).valuefloat());
    param = new al::ParameterVec3(id, group, value);
    break;
  }
  case ParameterDataType::PARAMETER_VEC4F: {
    al::Vec4f value(
        def.valuelist(0).valuefloat(), def.valuelist(1).valuefloat(),
        def.valuelist(2).valuefloat(), def.valuelist(3).valuefloat());
    param = new al::ParameterVec4(id, group, value);
    break;
  }
  case ParameterDataType::PARAMETER_COLORF: {
    al::Color value(
        def.valuelist(0).valuefloat(), def.valuelist(1).valuefloat(),
        def.valuelist(2).valuefloat(), def.valuelist(3).valuefloat());
    param = new al::ParameterColor(id, group, value);
    break;
  }
  case ParameterDataType::PARAMETER_POSED: {
    al::Pose value(
        {def.valuelist(0).valuedouble(), def.valuelist(1).valuedouble(),
         def.valuelist(2).valuedouble()},
        {def.valuelist(3).valuedouble(), def.valuelist(4).valuedouble(),
         def.valuelist(5).valuedouble(), def.valuelist(6).valuedouble()});
    param = new al::ParameterPose(id, group, value);
    break;
  }
  case ParameterDataType::PARAMETER_MENU:
    param = new al::ParameterMenu(id, group, def.valueint32());
    break;
  case ParameterDataType::PARAMETER_CHOICE:
    param = new al::ParameterChoice(id, group, def.valueuint64());
    break;
  case ParameterDataType::PARAMETER_TRIGGER:
    param = new al::Trigger(id, group);
    break;
  default:
    std::cerr << __FUNCTION__ << ": Invalid ParameterDataType" << std::endl;
    break;
  }

  if (param) {
    mLocalPSDs.emplace_back(
        std::make_unique<ParameterSpaceDimension>(param, true));
    registerParameterSpaceDimension(*mLocalPSDs.back(), src);
    delete param;
    return true;
  }

  return false;
}

bool TincProtocol::processRegisterParameterSpace(void *any, al::Socket *src) {
  // FIXME implement
  std::cerr << __FUNCTION__ << ": not implemented" << std::endl;
  return true;
}

bool TincProtocol::processRegisterProcessor(void *any, al::Socket *src) {
  // FIXME implement
  return true;
}

bool TincProtocol::processRegisterDiskBuffer(void *any, al::Socket *src) {
  // FIXME implement
  return true;
}

bool TincProtocol::processRegisterDataPool(void *any, al::Socket *src) {
  // FIXME implement
  return true;
}

void TincProtocol::sendRegisterMessage(ParameterSpaceDimension *dim,
                                       al::Socket *dst, al::Socket *src) {

  TincMessage msg = createRegisterParameterMessage(dim->getParameterMeta());
  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendRegisterMessage(ParameterSpace *ps, al::Socket *dst,
                                       al::Socket *src) {
  auto msg = createRegisterParameterSpaceMessage(ps);
  if (src) {
    sendTincMessage(&msg, nullptr, src->valueSource());
  } else {
    sendTincMessage(&msg);
  }

  for (auto dim : ps->getDimensions()) {
    sendRegisterMessage(dim.get(), dst, src);
    sendConfigureParameterSpaceAddDimension(ps, dim.get(), dst, src);
  }
}

void TincProtocol::sendRegisterMessage(Processor *p, al::Socket *dst,
                                       al::Socket *src) {
  // Handle Asynchronous Processors
  if (strcmp(typeid(*p).name(), typeid(ProcessorAsyncWrapper).name()) == 0) {
    p = dynamic_cast<ProcessorAsyncWrapper *>(p)->processor();
  }

  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::PROCESSOR);

  RegisterProcessor registerProcMessage;
  registerProcMessage.set_id(p->getId());
  if (strcmp(typeid(*p).name(), typeid(ProcessorScript).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::DATASCRIPT);
  } else if (strcmp(typeid(*p).name(), typeid(ProcessorGraph).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::CHAIN);
  } else if (strcmp(typeid(*p).name(), typeid(ProcessorCpp).name()) == 0) {
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

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(registerProcMessage);
  msg.set_allocated_details(detailsAny);

  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }

  if (dynamic_cast<ProcessorGraph *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ProcessorGraph *>(p)->getProcessors()) {
      sendRegisterMessage(childProcessor, dst, src);
    }
  }
}

void TincProtocol::sendRegisterMessage(DiskBufferAbstract *db, al::Socket *dst,
                                       al::Socket *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::DISK_BUFFER);

  RegisterDiskBuffer details;
  details.set_id(db->getId());

  DiskBufferType type = DiskBufferType::BINARY;

  if (strcmp(typeid(db).name(), typeid(DiskBufferNetCDFDouble).name()) == 0) {
    type = DiskBufferType::NETCDF;
  } else if (strcmp(typeid(db).name(), typeid(DiskBufferImage).name()) == 0) {
    type = DiskBufferType::IMAGE;
  } else if (strcmp(typeid(db).name(), typeid(DiskBufferJson).name()) == 0) {
    type = DiskBufferType::JSON;
  }
  details.set_type(type);
  details.set_basefilename(db->getBaseFileName());
  // TODO prepend node's root directory
  std::string path = al::File::currentPath() + db->getPath();
  details.set_path(path);

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);

  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendRegisterMessage(DataPool *dp, al::Socket *dst,
                                       al::Socket *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::DATA_POOL);

  RegisterDataPool details;
  details.set_id(dp->getId());
  details.set_parameterspaceid(dp->getParameterSpace().getId());
  details.set_cachedirectory(dp->getCacheDirectory());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);

  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }

  sendRegisterMessage(&dp->getParameterSpace(), dst, src);
}

bool TincProtocol::readConfigureMessage(int objectType, void *any,
                                        al::Socket *src) {
  switch (objectType) {
  case ObjectType::PARAMETER:
    return processConfigureParameter(any, src);
  case ObjectType::PROCESSOR:
    //    return sendProcessors(src);
    break;
  case ObjectType::DISK_BUFFER:
    return processConfigureDiskBuffer(any, src);
  case ObjectType::DATA_POOL:
    //    return sendDataPools(src);
    break;
  case ObjectType::PARAMETER_SPACE:
    return processConfigureParameterSpace(any, src);
  default:
    std::cerr << __FUNCTION__ << ": Invalid ObjectType" << std::endl;
    break;
  }
  return false;
}

bool TincProtocol::processConfigureParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<ConfigureParameter>()) {
    std::cerr << __FUNCTION__ << ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }

  ConfigureParameter conf;
  details->UnpackTo(&conf);
  auto addr = conf.id();

  for (auto *dim : mParameterSpaceDimensions) {
    if (addr == dim->getFullAddress()) {
      return processConfigureParameterMessage(conf, dim, src);
    }
  }

  for (auto *ps : mParameterSpaces) {
    for (auto dim : ps->getDimensions()) {
      if (addr == dim->getFullAddress()) {
        return processConfigureParameterMessage(conf, dim.get(), src);
      }
    }
  }

  std::cerr << __FUNCTION__ << ": Unable to find Parameter " << addr
            << std::endl;
  return false;
}

bool TincProtocol::processConfigureParameterSpace(void *any, al::Socket *src) {
  // FIXME implement
  std::cerr << __FUNCTION__ << ": not implemented" << std::endl;
  return true;
}

bool TincProtocol::processConfigureProcessor(void *any, al::Socket *src) {
  // FIXME implement
  return true;
}

bool TincProtocol::processConfigureDiskBuffer(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<ConfigureDiskBuffer>()) {
    std::cerr << __FUNCTION__ << ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }

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
            std::cerr << __FUNCTION__ << ": Error updating DiskBuffer " << id
                      << " from file " << file.valuestring() << std::endl;
            return false;
          }
          std::cout << "DiskBuffer " << id << " successfully loaded "
                    << file.valuestring() << std::endl;
          return true;
        }
      }

      std::cerr << __FUNCTION__ << ": Error configuring DiskBuffer " << id
                << std::endl;
      return false;
    }
  }

  std::cerr << __FUNCTION__ << ": Unable to find DiskBuffer " << id
            << std::endl;
  return false;
}

bool TincProtocol::processConfigureDataPool(void *any, al::Socket *src) {
  // FIXME implement
  return true;
}

void TincProtocol::sendConfigureMessage(ParameterSpaceDimension *dim,
                                        al::Socket *dst, al::Socket *src) {
  auto msgs = createConfigureParameterSpaceDimensionMessage(dim);
  for (auto &msg : msgs) {
    if (src) {
      sendTincMessage(&msg, dst, src->valueSource());
    } else {
      sendTincMessage(&msg, dst);
    }
  }
}

void TincProtocol::sendConfigureMessage(ParameterSpace *ps, al::Socket *dst,
                                        al::Socket *src) {
  // FIXME currently no config message needs to be sent for PS itself
  for (auto dim : ps->getDimensions()) {
    sendConfigureMessage(dim.get(), dst, src);
  }
}

void TincProtocol::sendConfigureMessage(Processor *p, al::Socket *dst,
                                        al::Socket *src) {
  // if processor is asynchronous
  if (strcmp(typeid(*p).name(), typeid(ProcessorAsyncWrapper).name()) == 0) {
    p = dynamic_cast<ProcessorAsyncWrapper *>(p)->processor();
  }

  for (auto config : p->configuration) {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PROCESSOR);
    ConfigureProcessor confMessage;
    confMessage.set_id(p->getId());
    confMessage.set_configurationkey(config.first);
    google::protobuf::Any *configValue = confMessage.configurationvalue().New();
    ParameterValue val;
    if (config.second.type == VARIANT_DOUBLE ||
        config.second.type == VARIANT_FLOAT) {
      val.set_valuedouble(config.second.valueDouble);
    } else if (config.second.type == VARIANT_INT64 ||
               config.second.type == VARIANT_INT32) {
      val.set_valueint64(config.second.valueInt64);
    } else if (config.second.type == VARIANT_STRING) {
      val.set_valuestring(config.second.valueStr);
    }
    configValue->PackFrom(val);
    auto details = msg.details().New();
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);

    if (src) {
      sendTincMessage(&msg, dst, src->valueSource());
    } else {
      sendTincMessage(&msg, dst);
    }
  }

  if (dynamic_cast<ProcessorGraph *>(p)) {
    for (auto childProcessor :
         dynamic_cast<ProcessorGraph *>(p)->getProcessors()) {
      if (src) {
        sendConfigureMessage(childProcessor, dst, src);
      } else {
        sendConfigureMessage(childProcessor, dst);
      }
    }
  }
}

void TincProtocol::sendConfigureMessage(DiskBufferAbstract *p, al::Socket *dst,
                                        al::Socket *src) {
  // TODO implement
}

void TincProtocol::sendConfigureMessage(DataPool *p, al::Socket *dst,
                                        al::Socket *src) {
  auto msg = createConfigureDataPoolMessage(p);
  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendConfigureParameterSpaceAddDimension(
    ParameterSpace *ps, ParameterSpaceDimension *dim, al::Socket *dst,
    al::Socket *src) {
  auto msg = createConfigureParameterSpaceAdd(ps, dim);
  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendConfigureParameterSpaceRemoveDimension(
    ParameterSpace *ps, ParameterSpaceDimension *dim, al::Socket *dst,
    al::Socket *src) {
  auto msg = createConfigureParameterSpaceRemove(ps, dim);

  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendValueMessage(float value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
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

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(bool value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  confMessage.set_id(fullAddress);

  confMessage.set_configurationkey(ParameterConfigureType::VALUE);
  ParameterValue val;
  val.set_valuebool(value);
  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(int32_t value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
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

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(uint64_t value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
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

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(std::string value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
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

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(al::Vec3f value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  confMessage.set_id(fullAddress);
  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

  ParameterValue val;
  ParameterValue *member = val.add_valuelist();
  member->set_valuefloat(value[0]);
  member = val.add_valuelist();
  member->set_valuefloat(value[1]);
  member = val.add_valuelist();
  member->set_valuefloat(value[2]);

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(al::Vec4f value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  confMessage.set_id(fullAddress);
  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

  ParameterValue val;
  ParameterValue *member = val.add_valuelist();
  member->set_valuefloat(value[0]);
  member = val.add_valuelist();
  member->set_valuefloat(value[1]);
  member = val.add_valuelist();
  member->set_valuefloat(value[2]);
  member = val.add_valuelist();
  member->set_valuefloat(value[3]);

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(al::Color value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  confMessage.set_id(fullAddress);
  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

  ParameterValue val;
  ParameterValue *member = val.add_valuelist();
  member->set_valuefloat(value.r);
  member = val.add_valuelist();
  member->set_valuefloat(value.g);
  member = val.add_valuelist();
  member->set_valuefloat(value.b);
  member = val.add_valuelist();
  member->set_valuefloat(value.a);

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendTincMessage(&msg, nullptr, src);
}

void TincProtocol::sendValueMessage(al::Pose value, std::string fullAddress,
                                    al::ValueSource *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER);
  google::protobuf::Any *details = msg.details().New();
  ConfigureParameter confMessage;
  confMessage.set_id(fullAddress);
  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

  ParameterValue val;
  ParameterValue *member = val.add_valuelist();
  member->set_valuedouble(value.pos()[0]);
  member = val.add_valuelist();
  member->set_valuedouble(value.pos()[1]);
  member = val.add_valuelist();
  member->set_valuedouble(value.pos()[2]);
  member = val.add_valuelist();
  member->set_valuedouble(value.quat()[0]);
  member = val.add_valuelist();
  member->set_valuedouble(value.quat()[1]);
  member = val.add_valuelist();
  member->set_valuedouble(value.quat()[2]);
  member = val.add_valuelist();
  member->set_valuedouble(value.quat()[3]);

  google::protobuf::Any *valueAny = confMessage.configurationvalue().New();
  valueAny->PackFrom(val);
  confMessage.set_allocated_configurationvalue(valueAny);

  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  sendTincMessage(&msg, nullptr, src);
}

bool TincProtocol::readCommandMessage(int objectType, void *any,
                                      al::Socket *src) {
  switch (objectType) {
  case ObjectType::PARAMETER:
    return processCommandParameter(any, src);
  case ObjectType::PROCESSOR:
    // return sendProcessors(src);
    break;
  case ObjectType::DISK_BUFFER:
    // return processConfigureDiskBuffer(any, src);
    break;
  case ObjectType::DATA_POOL:
    return processCommandDataPool(any, src);
  case ObjectType::PARAMETER_SPACE:
    return processCommandParameterSpace(any, src);
  default:
    std::cerr << __FUNCTION__ << ": Invalid ObjectType" << std::endl;
    break;
  }
  return false;
}

bool TincProtocol::sendCommandErrorMessage(uint64_t commandNumber,
                                           std::string objectId,
                                           std::string errorMessage,
                                           al::Socket *src) {

  TincMessage msg;
  msg.set_messagetype(MessageType::COMMAND_REPLY);
  msg.set_objecttype(ObjectType::PARAMETER);
  auto *msgDetails = msg.details().New();

  Command command;
  command.set_message_id(commandNumber);
  auto commandId = command.id();
  commandId.set_id(objectId);
  auto *errorPayload = command.details().New();
  CommandErrorPayload errorPayloadMsg;
  errorPayloadMsg.set_error(errorMessage);

  errorPayload->PackFrom(errorPayloadMsg);
  command.set_allocated_details(errorPayload);
  msg.set_allocated_details(msgDetails);
  return sendProtobufMessage(&msg, src);
}

bool TincProtocol::processCommandParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << __FUNCTION__ << ": Command message contains invalid payload"
              << std::endl;
    return false;
  }

  Command incomingCommand;
  details->UnpackTo(&incomingCommand);
  uint64_t commandNumber = incomingCommand.message_id();
  auto id = incomingCommand.id().id();
  if (incomingCommand.details().Is<ParameterRequestChoiceElements>()) {
    std::vector<std::string> elements;
    for (auto *ps : mParameterSpaces) {
      for (auto dim : ps->getDimensions()) {
        if (dim->getFullAddress() == id) {
          if (al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(
                  dim->getParameterMeta())) {
            elements = p->getElements();
            break;
          }
        }
      }
    }
    for (auto dim : mParameterSpaceDimensions) {
      if (dim->getFullAddress() == id) {
        if (al::ParameterChoice *p =
                dynamic_cast<al::ParameterChoice *>(dim->getParameterMeta())) {
          elements = p->getElements();
          break;
        }
      }
    }

    TincMessage msg;
    msg.set_messagetype(MessageType::COMMAND_REPLY);
    msg.set_objecttype(ObjectType::PARAMETER);
    auto *msgDetails = msg.details().New();

    Command command;
    command.set_message_id(commandNumber);
    auto commandId = command.id();
    commandId.set_id(id);

    auto *commandDetails = command.details().New();
    ParameterRequestChoiceElementsReply reply;
    for (auto &elem : elements) {
      reply.add_elements(elem);
    }

    commandDetails->PackFrom(reply);
    command.set_allocated_details(commandDetails);

    msgDetails->PackFrom(command);
    msg.set_allocated_details(msgDetails);

    // FIXME what is command reply? can this be sendTincmessage
    sendProtobufMessage(&msg, src);
    return true;
  } else {
    sendCommandErrorMessage(commandNumber, id,
                            "ParameterSpace not registered in server", src);
    return false;
  }
  return false;
}

bool TincProtocol::processCommandParameterSpace(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << __FUNCTION__ << ": Command message contains invalid payload"
              << std::endl;
    return false;
  }
  Command incomingCommand;
  details->UnpackTo(&incomingCommand);
  uint64_t commandNumber = incomingCommand.message_id();
  auto psId = incomingCommand.id().id();
  if (incomingCommand.details().Is<ParameterSpaceRequestCurrentPath>()) {
    ParameterSpaceRequestCurrentPath request;
    incomingCommand.details().UnpackTo(&request);
    for (auto ps : mParameterSpaces) {
      if (ps->getId() == psId) {
        auto curDir = ps->getCurrentRelativeRunPath();

        if (mVerbose) {
          std::cout << commandNumber << ":****: " << curDir << std::endl;
        }

        TincMessage msg;
        msg.set_messagetype(MessageType::COMMAND_REPLY);
        msg.set_objecttype(ObjectType::PARAMETER_SPACE);
        auto *msgDetails = msg.details().New();

        Command command;
        command.set_message_id(commandNumber);
        auto commandId = command.id();
        commandId.set_id(psId);

        auto *commandDetails = command.details().New();
        ParameterSpaceRequestCurrentPathReply reply;
        reply.set_path(curDir);

        commandDetails->PackFrom(reply);
        command.set_allocated_details(commandDetails);

        msgDetails->PackFrom(command);
        msg.set_allocated_details(msgDetails);
        sendTincMessage(&msg, src);
        return true;
      }
    }
    sendCommandErrorMessage(commandNumber, psId,
                            "ParameterSpace not registered in server", src);
  } else if (incomingCommand.details().Is<ParameterSpaceRequestRootPath>()) {
    ParameterSpaceRequestRootPath request;
    incomingCommand.details().UnpackTo(&request);
    for (auto ps : mParameterSpaces) {
      if (ps->getId() == psId) {
        auto rootPath = ps->getRootPath();

        if (mVerbose) {
          std::cout << commandNumber << ":****: " << rootPath << std::endl;
        }

        TincMessage msg;
        msg.set_messagetype(MessageType::COMMAND_REPLY);
        msg.set_objecttype(ObjectType::PARAMETER_SPACE);
        auto *msgDetails = msg.details().New();

        Command command;
        command.set_message_id(commandNumber);
        auto commandId = command.id();
        commandId.set_id(psId);

        auto *commandDetails = command.details().New();
        ParameterSpaceRequestRootPathReply reply;
        reply.set_path(rootPath);

        commandDetails->PackFrom(reply);
        command.set_allocated_details(commandDetails);

        msgDetails->PackFrom(command);
        msg.set_allocated_details(msgDetails);

        sendTincMessage(&msg, src);
        return true;
      }
    }
    sendCommandErrorMessage(commandNumber, psId,
                            "ParameterSpace not registered in server", src);
  } else {
    sendCommandErrorMessage(commandNumber, psId,
                            "Unsupported command reply for ParameterSpace",
                            src);
  }
  return false;
}

bool TincProtocol::processCommandDataPool(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << __FUNCTION__ << ": Command message contains invalid payload"
              << std::endl;
    return false;
  }

  Command incomingCommand;
  details->UnpackTo(&incomingCommand);
  uint64_t commandNumber = incomingCommand.message_id();
  auto datapoolId = incomingCommand.id().id();
  if (incomingCommand.details().Is<DataPoolCommandSlice>()) {
    DataPoolCommandSlice commandSlice;
    incomingCommand.details().UnpackTo(&commandSlice);
    auto field = commandSlice.field();

    std::vector<std::string> dims;
    dims.reserve(commandSlice.dimension_size());
    for (size_t i = 0; i < (size_t)commandSlice.dimension_size(); i++) {
      dims.push_back(commandSlice.dimension(i));
    }

    for (auto dp : mDataPools) {
      if (dp->getId() == datapoolId) {
        auto sliceName = dp->createDataSlice(field, dims);

        if (mVerbose) {
          std::cout << commandNumber << "::::: " << sliceName << std::endl;
        }

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

        // FIXME switch to sendTincMessage
        sendProtobufMessage(&msg, src);
        return true;
      }
    }
    sendCommandErrorMessage(commandNumber, datapoolId,
                            "Datapool not registered in server", src);
  } else if (incomingCommand.details().Is<DataPoolCommandCurrentFiles>()) {
    DataPoolCommandCurrentFiles commandSlice;
    incomingCommand.details().UnpackTo(&commandSlice);

    for (auto dp : mDataPools) {
      if (dp->getId() == datapoolId) {
        auto filenames = dp->getCurrentFiles();

        // FIXME uncomment?
        // if (mVerbose) {
        //   std::cout << commandNumber << "::::: " << sliceName << std::endl;
        // }

        TincMessage msg;
        msg.set_messagetype(MessageType::COMMAND_REPLY);
        msg.set_objecttype(ObjectType::DATA_POOL);
        auto *msgDetails = msg.details().New();

        Command command;
        command.set_message_id(commandNumber);
        auto commandId = command.id();
        commandId.set_id(datapoolId);

        auto *commandDetails = command.details().New();
        DataPoolCommandCurrentFilesReply reply;
        for (auto f : filenames) {
          reply.add_filenames(f);
        }

        commandDetails->PackFrom(reply);
        command.set_allocated_details(commandDetails);

        msgDetails->PackFrom(command);
        msg.set_allocated_details(msgDetails);

        // FIXME switch to sendTincMessage
        sendProtobufMessage(&msg, src);
        return true;
      }
    }
    sendCommandErrorMessage(commandNumber, datapoolId,
                            "Datapool not registered in server", src);
  } else {
    sendCommandErrorMessage(commandNumber, datapoolId,
                            "Unsupported command reply for Datapool", src);
  }
  return false;
}

bool TincProtocol::sendProtobufMessage(void *message, al::Socket *dst) {
  google::protobuf::Message &msg =
      *static_cast<google::protobuf::Message *>(message);
  size_t size = msg.ByteSizeLong();
  char *buffer = (char *)malloc(size + sizeof(size_t));
  memcpy(buffer, &size, sizeof(size_t));
  if (!msg.SerializeToArray(buffer + sizeof(size_t), size)) {
    free(buffer);
    std::cerr << __FUNCTION__ << ": Error serializing message" << std::endl;
  }

  if (mVerbose) {
    std::cout << __FUNCTION__ << ": Sending bytes " << size << std::endl;
  }
  auto bytes = dst->send(buffer, size + sizeof(size_t));
  if (bytes != size + sizeof(size_t)) {
    buffer[size + 1] = '\0';
    std::cerr << __FUNCTION__ << ": Error sending: " << buffer << " ("
              << strerror(errno) << ")" << std::endl;
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
