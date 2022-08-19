#include "tinc/DataPoolJson.hpp"
#include "tinc/DataPoolNetCDF.hpp"
#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDFData.hpp"
#include "tinc/DiskBufferText.hpp"
#include "tinc/ProcessorAsyncWrapper.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorGraph.hpp"
#include "tinc/TincClient.hpp"

#include "al/types/al_VariantValue.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;
using namespace tinc_protobuf;

namespace tinc_msg {
// TODO namespace these functions to avoid potential clashes
TincMessage createRegisterParameterMessage(al::ParameterMeta *param) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::PARAMETER);
  RegisterParameter details;
  details.set_id(param->getName());
  details.set_group(param->getGroup());
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_FLOAT);
    auto *def = details.mutable_defaultvalue();
    def->set_valuefloat(p->getDefault());
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_BOOL);
    auto *def = details.mutable_defaultvalue();
    def->set_valuefloat(p->getDefault());
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_STRING);
    auto *def = details.mutable_defaultvalue();
    def->set_valuestring(p->getDefault());
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_INT32);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint32(p->getDefault());
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_VEC3F);
    al::Vec3f defaultValue = p->getDefault();
    auto *def = details.mutable_defaultvalue();
    auto *val = def->add_valuelist();
    val->set_valuefloat(defaultValue[0]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[1]);
    val = def->add_valuelist();
    val->set_valuefloat(defaultValue[2]);
  } else if (al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_VEC4F);
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
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_COLORF);
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
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_POSED);
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
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_MENU);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint32(p->getDefault());
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_CHOICE);
    auto *def = details.mutable_defaultvalue();
    def->set_valueuint64(p->getDefault());
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_TRIGGER);
    auto *def = details.mutable_defaultvalue();
    def->set_valuebool(p->getDefault());
  } else if (al::ParameterInt64 *p =
                 dynamic_cast<al::ParameterInt64 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_INT64);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint64(p->getDefault());
  } else if (al::ParameterInt16 *p =
                 dynamic_cast<al::ParameterInt16 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_INT16);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint32(p->getDefault());
  } else if (al::ParameterInt8 *p = dynamic_cast<al::ParameterInt8 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_INT8);
    auto *def = details.mutable_defaultvalue();
    def->set_valueint32(p->getDefault());
  } else if (al::ParameterUInt64 *p =
                 dynamic_cast<al::ParameterUInt64 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_UINT64);
    auto *def = details.mutable_defaultvalue();
    def->set_valueuint64(p->getDefault());
  } else if (al::ParameterUInt32 *p =
                 dynamic_cast<al::ParameterUInt32 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_UINT32);
    auto *def = details.mutable_defaultvalue();
    def->set_valueuint32(p->getDefault());
  } else if (al::ParameterUInt16 *p =
                 dynamic_cast<al::ParameterUInt16 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_UINT16);
    auto *def = details.mutable_defaultvalue();
    def->set_valueuint32(p->getDefault());
  } else if (al::ParameterUInt8 *p =
                 dynamic_cast<al::ParameterUInt8 *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_UINT8);
    auto *def = details.mutable_defaultvalue();
    def->set_valueuint32(p->getDefault());
  } else if (al::ParameterDouble *p =
                 dynamic_cast<al::ParameterDouble *>(param)) {
    details.set_datatype(tinc_protobuf::ParameterType::PARAMETER_DOUBLE);
    auto *def = details.mutable_defaultvalue();
    def->set_valuedouble(p->getDefault());
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
  details.set_documentation(ps->getDocumentation());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  return msg;
}

TincMessage createPingMessage(uint64_t number) {
  TincMessage msg;
  msg.set_messagetype(MessageType::PING);
  msg.set_objecttype(ObjectType::GLOBAL);

  ParameterValue details;
  details.set_valueuint64(number);
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  return msg;
}

TincMessage createPongMessage(uint64_t number) {
  TincMessage msg;
  msg.set_messagetype(MessageType::PONG);
  msg.set_objecttype(ObjectType::GLOBAL);

  ParameterValue details;
  details.set_valueuint64(number);
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  return msg;
}

void createParameterValueMessage(al::ParameterMeta *param,
                                 ConfigureParameter &confMessage) {
  confMessage.set_id(param->getFullAddress());

  confMessage.set_configurationkey(ParameterConfigureType::VALUE);

  ParameterValue val;
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    val.set_valuefloat(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
  } else if (al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param)) {
    // FIXME ML change to bool
    val.set_valuefloat(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_BOOL));
  } else if (al::ParameterString *p =
                 dynamic_cast<al::ParameterString *>(param)) {
    val.set_valuestring(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
    val.set_valueint32(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
  } else if (al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param)) {
    al::Vec3f v = p->get();
    auto *member = val.add_valuelist();
    member->set_valuefloat(v[0]);
    member = val.add_valuelist();
    member->set_valuefloat(v[1]);
    member = val.add_valuelist();
    member->set_valuefloat(v[2]);

    val.set_nctype(
        static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
        static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
    val.set_nctype(
        static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
        static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
    val.set_nctype(
        static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
        static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
    val.set_nctype(
        static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
        static_cast<uint16_t>(al::VariantType::VARIANT_DOUBLE));
  } else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param)) {
    val.set_valueint32(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
  } else if (al::ParameterChoice *p =
                 dynamic_cast<al::ParameterChoice *>(param)) {
    val.set_valueuint64(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_UINT64));
  } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
    val.set_valuebool(p->get());
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_BOOL));
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
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
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
    val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
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
    if (dim->getSpaceDataType() == al::VariantType::VARIANT_FLOAT) {
      for (auto &value : dim->getSpaceValues<float>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valuefloat(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_DOUBLE) {
      for (auto &value : dim->getSpaceValues<double>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valuedouble(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_DOUBLE));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT8) {
      for (auto &value : dim->getSpaceValues<uint8_t>()) {
        // we utilize variable encoding of protobuf to handle 1 byte ints
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueuint32(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_UINT8));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT8) {
      for (auto &value : dim->getSpaceValues<int8_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueint32(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_INT8));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT16) {
      for (auto &value : dim->getSpaceValues<uint16_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueuint32(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_UINT16));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT16) {
      for (auto &value : dim->getSpaceValues<int16_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueint32(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_INT16));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT32) {
      for (auto &value : dim->getSpaceValues<uint32_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueuint32(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_UINT32));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT32) {
      for (auto &value : dim->getSpaceValues<int32_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueint32(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT64) {
      for (auto &value : dim->getSpaceValues<uint64_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueuint64(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_UINT64));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT64) {
      for (auto &value : dim->getSpaceValues<int64_t>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valueint64(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_INT64));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_BOOL) {
      for (auto &value : dim->getSpaceValues<float>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valuefloat(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_BOOL));
      }
    } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_STRING) {
      for (auto &value : dim->getSpaceValues<std::string>()) {
        auto *newValue = valuesMessage.add_values();
        newValue->set_valuestring(value);
        newValue->set_nctype(
            static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
      }
    } else {
      std::cerr << "Other types not supported yet" << std::endl;
    }
    // FIXME ML support the rest of the types.
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

std::vector<TincMessage>
createConfigureParameterSpaceMessage(ParameterSpace *ps) {
  std::vector<TincMessage> confMessages;

  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::PARAMETER_SPACE);

  ConfigureParameterSpace confMessage;
  confMessage.set_id(ps->getId());
  confMessage.set_configurationkey(
      ParameterSpaceConfigureType::CURRENT_TEMPLATE);

  ParameterValue valuesMessage;
  valuesMessage.set_valuestring(ps->getCurrentPathTemplate());
  valuesMessage.set_nctype(
      static_cast<uint16_t>(al::VariantType::VARIANT_STRING));

  auto confValue = confMessage.configurationvalue().New();
  confValue->PackFrom(valuesMessage);
  confMessage.set_allocated_configurationvalue(confValue);

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(confMessage);
  msg.set_allocated_details(detailsAny);
  confMessages.push_back(msg);

  {

    TincMessage msg2;
    msg2.set_messagetype(MessageType::CONFIGURE);
    msg2.set_objecttype(ObjectType::PARAMETER_SPACE);
    ConfigureParameterSpace confMessage2;
    confMessage2.set_id(ps->getId());
    confMessage2.set_configurationkey(ParameterSpaceConfigureType::ROOT_PATH);

    ParameterValue valuesMessage2;
    valuesMessage2.set_valuestring(ps->getRootPath());
    valuesMessage2.set_nctype(
        static_cast<uint16_t>(al::VariantType::VARIANT_STRING));

    auto confValue2 = confMessage2.configurationvalue().New();
    confValue2->PackFrom(valuesMessage2);
    confMessage2.set_allocated_configurationvalue(confValue2);

    google::protobuf::Any *detailsAny2 = msg2.details().New();
    detailsAny2->PackFrom(confMessage2);
    msg2.set_allocated_details(detailsAny2);
    confMessages.push_back(msg2);
  }

  {
    TincMessage msg2;
    msg2.set_messagetype(MessageType::CONFIGURE);
    msg2.set_objecttype(ObjectType::PARAMETER_SPACE);
    auto cm = ps->getCacheManager();
    if (cm) {
      ConfigureParameterSpace confMessage2;
      confMessage2.set_id(ps->getId());
      confMessage2.set_configurationkey(
          ParameterSpaceConfigureType::CACHE_PATH);

      tinc_protobuf::DistributedPath valuesMessage2;

      auto path = cm->getDistributedPath();
      valuesMessage2.set_filename(path.filename);
      valuesMessage2.set_relativepath(path.relativePath);
      valuesMessage2.set_rootpath(path.rootPath);
      valuesMessage2.set_protocolid(path.protocolId);

      auto confValue2 = confMessage2.configurationvalue().New();
      confValue2->PackFrom(valuesMessage2);
      confMessage2.set_allocated_configurationvalue(confValue2);

      google::protobuf::Any *detailsAny2 = msg2.details().New();
      detailsAny2->PackFrom(confMessage2);
      msg2.set_allocated_details(detailsAny2);
      confMessages.push_back(msg2);
    }
  }

  {
    TincMessage msg2;
    msg2.set_messagetype(MessageType::CONFIGURE);
    msg2.set_objecttype(ObjectType::PARAMETER_SPACE);
    ConfigureParameterSpace confMessage2;
    confMessage2.set_id(ps->getId());
    confMessage2.set_configurationkey(
        ParameterSpaceConfigureType::PS_DOCUMENTATION);

    auto confValue2 = confMessage2.configurationvalue().New();
    confValue2->set_value(ps->getDocumentation());
    confMessage2.set_allocated_configurationvalue(confValue2);

    google::protobuf::Any *detailsAny2 = msg2.details().New();
    detailsAny2->PackFrom(confMessage2);
    msg2.set_allocated_details(detailsAny2);
    confMessages.push_back(msg2);
  }

  return confMessages;
}

TincMessage createConfigureDiskBufferMessage(DiskBufferAbstract *dp) {
  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::DISK_BUFFER);
  ConfigureDiskBuffer confMessage;
  confMessage.set_id(dp->getId());
  confMessage.set_configurationkey(DiskBufferConfigureType::CURRENT_FILE);
  google::protobuf::Any *configValue = confMessage.configurationvalue().New();
  ParameterValue val;
  val.set_valuestring(dp->getCurrentFileName());
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
  configValue->PackFrom(val);
  confMessage.set_allocated_configurationvalue(configValue);
  auto details = msg.details().New();
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);
  return msg;
}

std::vector<TincMessage> createConfigureDataPoolMessage(DataPool *dp) {
  std::vector<TincMessage> msgs;

  TincMessage msg;
  msg.set_messagetype(MessageType::CONFIGURE);
  msg.set_objecttype(ObjectType::DATA_POOL);
  ConfigureDataPool confMessage;
  confMessage.set_id(dp->getId());
  confMessage.set_configurationkey(DataPoolConfigureType::SLICE_CACHE_DIR);
  google::protobuf::Any *configValue = confMessage.configurationvalue().New();
  ParameterValue val;
  val.set_valuestring(dp->getCacheDirectory());
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
  configValue->PackFrom(val);
  confMessage.set_allocated_configurationvalue(configValue);
  auto details = msg.details().New();
  details->PackFrom(confMessage);
  msg.set_allocated_details(details);

  msgs.push_back(msg);

  TincMessage msg2;
  msg2.set_messagetype(MessageType::CONFIGURE);
  msg2.set_objecttype(ObjectType::DATA_POOL);
  ConfigureDataPool confMessage2;
  confMessage2.set_id(dp->getId());
  confMessage2.set_configurationkey(DataPoolConfigureType::DATA_FILES);
  DataPoolDataFiles dataFiles;
  for (auto &df : dp->getRegisteredDataFiles()) {
    dataFiles.add_filename(df.first);
    dataFiles.add_dimension(df.second);
  }
  google::protobuf::Any *configValue2 = confMessage2.configurationvalue().New();
  configValue2->PackFrom(dataFiles);
  confMessage2.set_allocated_configurationvalue(configValue2);
  auto details2 = msg2.details().New();
  details2->PackFrom(confMessage2);
  msg2.set_allocated_details(details2);

  msgs.push_back(msg2);
  return msgs;
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
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

  //  std::cout << __FUNCTION__ << ": Configure parameter" << std::endl;
  if (!confValue.Is<ParameterValue>()) {
    std::cerr << __FUNCTION__ << ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }
  const ParameterConfigureType &command = conf.configurationkey();
  ParameterValue v;
  confValue.UnpackTo(&v);
  // TODO ML check and validate types in v.nctype()
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
    // TODO ML validate that v.nctype() is correct
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
} // namespace tinc_msg

using namespace tinc_msg;

bool TincProtocol::processConfigureParameterMessage(
    void *conf_, ParameterSpaceDimension *dim, al::Socket *src, bool forward) {
  auto &conf = *static_cast<ConfigureParameter *>(conf_);
  ParameterConfigureType command = conf.configurationkey();

  if (command == ParameterConfigureType::SPACE) {
    ParameterSpaceValues sv;
    if (conf.configurationvalue().Is<ParameterSpaceValues>()) {
      conf.configurationvalue().UnpackTo(&sv);
      auto values = sv.values();
      auto idsIt = sv.ids().begin();
      if (sv.ids().size() != 0 && sv.ids().size() != values.size()) {
        std::cerr << mMessagePrefix + " ids size mismatch. ignoring"
                  << std::endl;
        idsIt = sv.ids().end();
      }
      dim->clear(src);
      // TODO allow overriding the type of the space remotely?
      if (dim->getSpaceDataType() == al::VariantType::VARIANT_FLOAT) {
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

        dim->setSpaceValues(newValues, "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT8) {
        std::vector<int8_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueint32());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT8) {
        std::vector<uint8_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueuint32());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT16) {
        std::vector<int16_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueint32());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT16) {
        std::vector<uint16_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueuint32());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT32) {
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
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT32) {
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
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_INT64) {
        std::vector<int64_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueint64());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
      } else if (dim->getSpaceDataType() == al::VariantType::VARIANT_UINT64) {
        std::vector<uint64_t> newValues;
        std::vector<std::string> newIds;
        newValues.reserve(values.size());
        for (auto &v : values) {
          newValues.push_back(v.valueuint64());
          if (idsIt != sv.ids().end()) {
            newIds.push_back(*idsIt);
            idsIt++;
          }
        }
        dim->setSpaceValues(newValues.data(), newValues.size(), "", src);
        dim->setSpaceIds(newIds, src);
        if (newIds.size() != 0 && newValues.size() != 0) {
          dim->conformSpace(src);
        }
        // FIXME add all types
      } else {
        return false;
      }
      if (forward) {
        sendConfigureMessage(dim, nullptr, src);
      }
      return true;
    }
  } else if (command == ParameterConfigureType::SPACE_TYPE) {

    if (conf.configurationvalue().Is<ParameterValue>()) {
      ParameterValue v;
      conf.configurationvalue().UnpackTo(&v);
      dim->setSpaceRepresentationType(
          (ParameterSpaceDimension::RepresentationType)v.valueint32(), src);

      if (forward) {
        sendConfigureMessage(dim, nullptr, src);
      }
      return true;

    } else {
      return false;
    }
  }
  al::ParameterMeta *param = dim->getParameterMeta();
  return processConfigureParameterValueMessage(conf, param, src);
}

//// ------------------------------------------------------
bool TincProtocol::registerParameter(al::ParameterMeta &pmeta,
                                     al::Socket *src) {
  for (auto *dim : mParameterSpaceDimensions) {
    if (dim->getName() == pmeta.getName() &&
        dim->getGroup() == pmeta.getGroup()) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": Parameter " +
                         pmeta.getName() + " (Group: " + pmeta.getGroup() +
                         ") already registered."
                  << std::endl;
      }
      if (&pmeta != dim->getParameterMeta()) {
        std::cerr << mMessagePrefix + __FUNCTION__ +
                         ": Parameter is already registered but pointer "
                         "doesn't match."
                  << std::endl;
      }
      return false;
    }
  }
  mLocalPSDs.emplace_back(
      std::make_shared<ParameterSpaceDimension>(&pmeta, false));
  registerParameterSpaceDimension(*mLocalPSDs.back(), src);
  return true;
}

bool TincProtocol::registerParameterSpaceDimension(ParameterSpaceDimension &psd,
                                                   al::Socket *src) {
  for (auto *dim : mParameterSpaceDimensions) {
    if (dim->getFullAddress() == psd.getFullAddress()) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ +
                         ": ParameterSpaceDimension " + psd.getFullAddress() +
                         " already registered."
                  << std::endl;
      }
      if (dim != &psd) {
        std::cerr
            << mMessagePrefix + __FILE__ +
                   " ERROR: Attempted to register a  dimension that already "
                   "exists. Dimension not registered."
            << std::endl;
        assert(dim == &psd);
      }
      return false;
    }
  }

  for (auto *ps : mParameterSpaces) {
    for (auto *dim : ps->getDimensions()) {
      if (dim == &psd || dim->getFullAddress() == psd.getFullAddress()) {
        if (&psd != dim) {
          std::cerr
              << mMessagePrefix + __FILE__ +
                     " ERROR: Attempted to register a dimension that already "
                     "exists in a ParameterSpace with a different object. "
                     "Duplicate dimension will cause inconsistent state."
              << std::endl;
          assert(&psd == dim); // Forced failed assert
          return false;
        }
      }
    }
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + __FUNCTION__ + ": Register new dimension  " +
                     psd.getName()
              << std::endl;
  }
  mParameterSpaceDimensions.push_back(&psd);
  connectParameterCallbacks(*psd.getParameterMeta());
  connectDimensionCallbacks(psd);

  // Broadcast registered ParameterSpaceDimension
  sendRegisterMessage(&psd, nullptr, src);
  sendConfigureMessage(&psd, nullptr, src);
  return true;
}

bool TincProtocol::registerParameterSpace(ParameterSpace &ps, al::Socket *src) {
  for (auto *p : mParameterSpaces) {
    if (p->getId() == ps.getId()) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": ParameterSpace " +
                         ps.getId() + " already registered."
                  << std::endl;
      }
      if (p != &ps) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": ERROR ParameterSpace " +
                         ps.getId() + " already registered as another object."
                  << std::endl;
      }
      assert(p == &ps);
      return false;
    }
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + __FUNCTION__ +
                     ": Registering new ParameterSpace: " + ps.getId()
              << std::endl;
  }

  mParameterSpaces.push_back(&ps);

  ps.onDimensionRegister = [this](ParameterSpaceDimension *changedDimension,
                                  ParameterSpace *ps, al::Socket *src) {
    if (mVerbose) {
      std::cout << mMessagePrefix + __FUNCTION__ +
                       ": Callback onDimensionRegister() " + ps->getId()
                << std::endl;
    }

    // function that invokes this cb should do relevant checks
    // connects callbacks, sends register + configure messages
    registerParameterSpaceDimension(*changedDimension, src);
    sendConfigureParameterSpaceAddDimension(ps, changedDimension, nullptr, src);
  };

  ps.onDimensionRemove = [this](ParameterSpaceDimension *changedDimension,
                                ParameterSpace *ps, bool invoked,
                                al::Socket *src) {
    sendConfigureParameterSpaceRemoveDimension(ps, changedDimension, nullptr,
                                               src);
    if (!invoked) {
      removeParameter(changedDimension->getName(), changedDimension->getGroup(),
                      true, src);
    }
  };
  ps.modified = [&](al::Socket *src = nullptr) {
    auto msgs = createConfigureParameterSpaceMessage(&ps);
    for (auto &msg : msgs) {
      if (src) {
        sendTincMessage(&msg, nullptr, src->valueSource());
      } else {
        sendTincMessage(&msg, nullptr, nullptr);
      }
    }
  };

  // register PSDs attached to the ParameterSpace
  for (auto *dim : ps.getDimensions()) {
    registerParameterSpaceDimension(*dim, src);
  }

  // Broadcast registered ParameterSpace
  sendRegisterMessage(&ps, nullptr, src);
  sendConfigureMessage(&ps, nullptr, src);

  return true;
}

bool TincProtocol::registerProcessor(Processor &processor, al::Socket *src) {
  for (auto *p : mProcessors) {
    if (p->getId() == processor.getId()) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": Processor " +
                         processor.getId() + " already registered."
                  << std::endl;
      }
      if (p != &processor) {
        std::cerr << mMessagePrefix + __FUNCTION__ + ": ERROR Processor " +
                         processor.getId() +
                         " already registered as a different object."
                  << std::endl;
      }
      assert(p == &processor);
      return false;
    }
  }
  mProcessors.push_back(&processor);
  // FIXME we should register parameters registered with processors.
  // Look at how it is done in ParameterSpace that also handle internal
  // parameters
  //    for (auto *param: processor.parameters()) {
  //    }

  // Broadcast registered processor
  sendRegisterMessage(&processor, nullptr, src);
  sendConfigureMessage(&processor, nullptr, src);
  return true;
}

bool TincProtocol::registerDiskBuffer(DiskBufferAbstract &db, al::Socket *src) {
  for (auto *p : mDiskBuffers) {
    if (p->getId() == db.getId()) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": DiskBuffer " +
                         db.getId() + " already registered."
                  << std::endl;
      }
      if (p != &db) {
        std::cerr << mMessagePrefix + __FUNCTION__ + ": ERROR Diskbuffer " +
                         db.getId() +
                         " already registered as a different object."
                  << std::endl;
      }
      assert(p == &db);
      return false;
    }
  }
  mDiskBuffers.push_back(&db);

  db.registerNotifyCallback([&](bool ok) {
    if (ok) {
      // TODO send more granular messages, not everything.
      sendConfigureMessage(&db, nullptr, nullptr);
    }
  });

  // Broadcast registered DiskBuffer
  sendRegisterMessage(&db, nullptr, src);
  sendConfigureMessage(&db, nullptr, src);
  return true;
}

bool TincProtocol::registerDataPool(DataPool &dp, al::Socket *src) {
  for (auto *p : mDataPools) {
    if (p->getId() == dp.getId()) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": DataPool " +
                         dp.getId() + " already registered."
                  << std::endl;
      }
      if (p != &dp) {
        std::cerr << mMessagePrefix + __FUNCTION__ + ": ERROR DataPool " +
                         dp.getId() +
                         " already registered as a different object."
                  << std::endl;
      }
      assert(p == &dp);
      return false;
    }
  }
  mDataPools.push_back(&dp);
  // FIXME check register order datapool -> ps
  registerParameterSpace(dp.getParameterSpace(), src);

  dp.modified = [&](al::Socket *src = nullptr) {
    auto msgs = createConfigureDataPoolMessage(&dp);
    for (auto &msg : msgs) {
      if (src) {
        sendTincMessage(&msg, nullptr, src->valueSource());
      } else {
        sendTincMessage(&msg, nullptr, nullptr);
      }
    }
  };

  // Broadcast registered DataPool and dependencies

  for (auto *p : dp.getParameterSpace().getDimensions()) {
    sendRegisterMessage(p, nullptr, src);
  }
  sendRegisterMessage(&(dp.getParameterSpace()), nullptr, src);

  sendRegisterMessage(&dp, nullptr, src);
  sendConfigureMessage(&dp, nullptr, src);
  return true;
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

void TincProtocol::removeParameter(std::string name, std::string group,
                                   bool invoked, al::Socket *src) {
  // FIXME add lock

  for (auto it = mParameterSpaceDimensions.begin();
       it != mParameterSpaceDimensions.end();) {
    if (((*it)->getName() == name && (*it)->getGroup() == group) ||
        (group == "" && (*it)->getFullAddress() == name)) {

      sendRemoveMessage(*it, nullptr, src);

      it = mParameterSpaceDimensions.erase(it);

      if (!invoked) {
        for (auto *ps : mParameterSpaces) {
          ps->removeDimension(name, group, true, src);
        }
      }

      for (auto itLocal = mLocalPSDs.begin(); itLocal != mLocalPSDs.end();) {
        if (((*itLocal)->getName() == name &&
             (*itLocal)->getGroup() == group) ||
            (group == "" && (*itLocal)->getFullAddress() == name)) {

          if (mVerbose) {
            std::cout << mMessagePrefix + __FUNCTION__ +
                             ": ParameterSpaceDimension " +
                             (*itLocal)->getFullAddress() + " removed."
                      << std::endl;
          }

          itLocal = mLocalPSDs.erase(itLocal);

          return; // removed local psd & stored ptr
        } else {
          ++itLocal;
        }
      }

      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ +
                         ": ParameterSpaceDimension " + name +
                         " (Group: " + group +
                         ") removed from TincProtocol but original is stored "
                         "externally."
                  << std::endl;
      }
      return; // removed stored ptr but psd is owned externally
    } else {
      ++it;
    }
  }

  std::cerr << mMessagePrefix + __FUNCTION__ +
                   ": Unable to find ParameterSpaceDimension " + name +
                   " (Group: " + group + ")"
            << std::endl;
}

ParameterSpaceDimension *TincProtocol::getDimension(std::string name,
                                                    std::string group) {
  for (auto *dim : mParameterSpaceDimensions) {
    if (dim->getName() == name && dim->getGroup() == group) {
      return dim;
    } else if (group == "" && dim->getFullAddress() == name) {
      return dim;
    }
  }
  for (auto *ps : mParameterSpaces) {
    auto dim = ps->getDimension(name, group);
    if (dim) {
      return dim;
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

ParameterSpace *TincProtocol::getParameterSpace(std::string name) {
  for (auto *ps : mParameterSpaces) {
    if (ps->getId() == name) {
      return ps;
    }
  }
  return nullptr;
}

DiskBufferAbstract *TincProtocol::getDiskBuffer(std::string name) {
  for (auto *db : mDiskBuffers) {
    if (db->getId() == name) {
      return db;
    }
  }
  return nullptr;
}

DataPool *TincProtocol::getDataPool(std::string name) {
  for (auto *dp : mDataPools) {
    if (dp->getId() == name) {
      return dp;
    }
  }
  return nullptr;
}

Processor *TincProtocol::getProcessor(std::string name) {
  for (auto *dp : mProcessors) {
    if (dp->getId() == name) {
      return dp;
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
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     "ERROR: Can't mark as available. Not busy."
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

void TincProtocol::print(std::ostream &stream) {
  stream << "Mapped paths: " << std::endl;
  for (const auto &p : mRootPathMap) {
    stream << p.first << " -> ";
    for (const auto &m : p.second) {
      stream << m.first << " ::: " << m.second << std::endl << "          ";
    }
  }
  stream << std::endl;
  stream << " --- *** ParameterSpaceDimensions *** ---" << std::endl;
  for (auto dim : mParameterSpaceDimensions) {
    dim->print(stream);
  }
  for (auto ps : mParameterSpaces) {
    ps->print(stream);
  }
}

void TincProtocol::connectParameterCallbacks(al::ParameterMeta &pmeta) {
  if (al::Parameter *p = dynamic_cast<al::Parameter *>(&pmeta)) {
    p->registerChangeCallback([&, p](float value, al::ValueSource *src) {
      sendValueMessage(value, p->getFullAddress(), src);
    });
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    p->registerMetaChangeCallback([&](al::ValueSource *src) {
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
    std::cout << mMessagePrefix + __FUNCTION__ +
                     ": Ignoring object id. Sending all requested objects."
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
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Invalid ObjectType"
              << std::endl;
    break;
  }
}

void TincProtocol::processRequestParameters(al::Socket *dst) {

  if (mVerbose) {
    std::cout << mMessagePrefix + "Protocol received Request Parameters"
              << std::endl;
  }
  for (auto *dim : mParameterSpaceDimensions) {
    sendRegisterMessage(dim, dst);
    sendConfigureMessage(dim, dst);
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + "done sending Parameters" << std::endl;
  }
}

void TincProtocol::processRequestParameterSpaces(al::Socket *dst) {
  if (mVerbose) {
    std::cout << mMessagePrefix + "Protocol received Request ParameterSpaces"
              << std::endl;
  }
  for (auto &ps : mParameterSpaces) {
    sendRegisterMessage(ps, dst);
    sendConfigureMessage(ps, dst);
    for (auto &dim : ps->getDimensions()) {
      sendRegisterMessage(dim, dst);
      sendConfigureMessage(dim, dst);
      sendConfigureParameterSpaceAddDimension(ps, dim, dst);
    }
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + "done sending ParameterSpaces" << std::endl;
  }
}

void TincProtocol::processRequestProcessors(al::Socket *dst) {
  if (mVerbose) {
    std::cout << mMessagePrefix + "Protocol received Request Processors"
              << std::endl;
  }
  for (auto *p : mProcessors) {
    sendRegisterMessage(p, dst);
    sendConfigureMessage(p, dst);
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + "done sending Processors" << std::endl;
  }
}

void TincProtocol::processRequestDiskBuffers(al::Socket *dst) {
  if (mVerbose) {
    std::cout << mMessagePrefix + "Protocol received Request DiskBuffers"
              << std::endl;
  }
  for (auto *db : mDiskBuffers) {
    sendRegisterMessage(db, dst);
    sendConfigureMessage(db, dst);
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + "done sending DiskBuffers" << std::endl;
  }
}

void TincProtocol::processRequestDataPools(al::Socket *dst) {
  if (mVerbose) {
    std::cout << mMessagePrefix + "Protocol received Request DataPools"
              << std::endl;
  }
  for (auto *dp : mDataPools) {
    sendRegisterMessage(dp, dst);
    sendConfigureMessage(dp, dst);
  }
  if (mVerbose) {
    std::cout << mMessagePrefix + "done sending DataPools" << std::endl;
  }
}

bool TincProtocol::readRegisterMessage(int objectType, void *any,
                                       al::Socket *src, bool forward) {
  switch (objectType) {
  case ObjectType::PARAMETER:
    return processRegisterParameter(any, src, forward);
  case ObjectType::PROCESSOR:
    return processRegisterProcessor(any, src, forward);
  case ObjectType::DISK_BUFFER:
    return processRegisterDiskBuffer(any, src, forward);
  case ObjectType::DATA_POOL:
    return processRegisterDataPool(any, src, forward);
  case ObjectType::PARAMETER_SPACE:
    return processRegisterParameterSpace(any, src, forward);
  default:
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Invalid ObjectType"
              << std::endl;
    break;
  }
  return false;
}

bool TincProtocol::processRegisterParameter(void *any, al::Socket *src,
                                            bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterParameter>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Register message contains invalid payload"
              << std::endl;
    return false;
  }

  RegisterParameter command;
  details->UnpackTo(&command);
  auto id = command.id();
  auto group = command.group();
  auto def = command.defaultvalue();
  auto datatype = command.datatype();

  if (mVerbose) {
    std::cout << mMessagePrefix + "Registering Parameter " + id +
                     " (Group: " + group + ")"
              << std::endl;
  }

  for (auto &dim : dimensions()) {
    if (dim->getName() == id && dim->getGroup() == group) {
      if (mVerbose) {
        // FIXME apply configuration (min, max, default) if found
        std::cout << mMessagePrefix + __FUNCTION__ + ": Parameter " + id +
                         " (Group: " + group + ") already registered."
                  << std::endl;
      }
      return true;
    }
  }
  for (auto *ps : mParameterSpaces) {
    for (auto *dim : ps->getDimensions()) {
      if (dim->getName() == id && dim->getGroup() == group) {
        if (mVerbose) {
          // FIXME apply configuration (min, max, default) if found
          std::cout << mMessagePrefix + __FUNCTION__ + ": Parameter " + id +
                           " (Group: " + group +
                           ") already registered in a parameter space."
                    << std::endl;
        }
        return true;
      }
    }
  }

  al::ParameterMeta *param = nullptr;
  switch ((tinc_protobuf::ParameterType)datatype) {
  case tinc_protobuf::ParameterType::PARAMETER_FLOAT:
    param = new al::Parameter(id, group, def.valuefloat());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_BOOL:
    param = new al::ParameterBool(id, group, def.valuefloat());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_STRING:
    param = new al::ParameterString(id, group, def.valuestring());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_INT32:
    param = new al::ParameterInt(id, group, def.valueint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_VEC3F: {
    al::Vec3f value(def.valuelist(0).valuefloat(),
                    def.valuelist(1).valuefloat(),
                    def.valuelist(2).valuefloat());
    param = new al::ParameterVec3(id, group, value);
    break;
  }
  case tinc_protobuf::ParameterType::PARAMETER_VEC4F: {
    al::Vec4f value(
        def.valuelist(0).valuefloat(), def.valuelist(1).valuefloat(),
        def.valuelist(2).valuefloat(), def.valuelist(3).valuefloat());
    param = new al::ParameterVec4(id, group, value);
    break;
  }
  case tinc_protobuf::ParameterType::PARAMETER_COLORF: {
    al::Color value(
        def.valuelist(0).valuefloat(), def.valuelist(1).valuefloat(),
        def.valuelist(2).valuefloat(), def.valuelist(3).valuefloat());
    param = new al::ParameterColor(id, group, value);
    break;
  }
  case tinc_protobuf::ParameterType::PARAMETER_POSED: {
    al::Pose value(
        {def.valuelist(0).valuedouble(), def.valuelist(1).valuedouble(),
         def.valuelist(2).valuedouble()},
        {def.valuelist(3).valuedouble(), def.valuelist(4).valuedouble(),
         def.valuelist(5).valuedouble(), def.valuelist(6).valuedouble()});
    param = new al::ParameterPose(id, group, value);
    break;
  }
  case tinc_protobuf::ParameterType::PARAMETER_MENU:
    param = new al::ParameterMenu(id, group, def.valueint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_CHOICE:
    param = new al::ParameterChoice(id, group, def.valueuint64());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_TRIGGER:
    param = new al::Trigger(id, group);
    break;
  case tinc_protobuf::ParameterType::PARAMETER_INT64:
    param = new al::ParameterInt64(id, group, def.valueint64());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_INT16:
    param = new al::ParameterInt16(id, group, def.valueint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_INT8:
    param = new al::ParameterInt8(id, group, def.valueint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_UINT64:
    param = new al::ParameterUInt64(id, group, def.valueuint64());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_UINT32:
    param = new al::ParameterUInt32(id, group, def.valueuint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_UINT16:
    param = new al::ParameterUInt16(id, group, def.valueuint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_UINT8:
    param = new al::ParameterUInt8(id, group, def.valueuint32());
    break;
  case tinc_protobuf::ParameterType::PARAMETER_DOUBLE:
    param = new al::ParameterDouble(id, group, def.valuedouble());
    break;
  default:
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Invalid ParameterType"
              << std::endl;
    break;
  }

  if (param) {
    mLocalPSDs.emplace_back(
        std::make_shared<ParameterSpaceDimension>(param, true));

    if (registerParameterSpaceDimension(*mLocalPSDs.back(), src)) {
      if (forward) {
        sendRegisterMessage(mLocalPSDs.back().get(), nullptr, src);
      }
    }
    delete param;
    return true;
  }

  return false;
}

bool TincProtocol::processRegisterParameterSpace(void *any, al::Socket *src,
                                                 bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterParameterSpace>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Register message contains invalid payload"
              << std::endl;
    return false;
  }

  RegisterParameterSpace command;
  details->UnpackTo(&command);
  auto id = command.id();

  if (mVerbose) {
    std::cout << mMessagePrefix + "Registering ParameterSpace " + id
              << std::endl;
  }

  for (auto &ps : mParameterSpaces) {
    if (ps->getId() == id) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": ParameterSpace " + id +
                         " already registered."
                  << std::endl;
      }
      return true;
    }
  }

  mLocalPSs.emplace_back(std::make_shared<ParameterSpace>(id));
  mLocalPSs.back()->setDocumentation(command.documentation());
  mLocalPSs.back()->setVerbose(mVerbose); // Inherit verbosity
  if (registerParameterSpace(*mLocalPSs.back(), src)) {
    if (forward) {
      sendRegisterMessage(mLocalPSs.back().get(), nullptr, src);
    }
  }

  return true;
}

bool TincProtocol::processRegisterProcessor(void *any, al::Socket *src,
                                            bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterProcessor>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Register Processor message contains invalid payload"
              << std::endl;
    return false;
  }

  RegisterProcessor command;
  details->UnpackTo(&command);
  auto id = command.id();

  if (mVerbose) {
    std::cout << mMessagePrefix + "Registering Processor " + id << std::endl;
  }

  for (auto &proc : mProcessors) {
    if (proc->getId() == id) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": Processor " + id +
                         " already registered."
                  << std::endl;
      }
      // TODO verify consistency
      return true;
    }
  }

  auto type = command.type();
  if (type == DATASCRIPT) {
    mLocalProcs.emplace_back(std::make_shared<ProcessorScript>(id));
  } else if (type == CHAIN) {
    mLocalProcs.emplace_back(std::make_shared<ProcessorGraph>(id));
  } else if (type == CPP) {
    mLocalProcs.emplace_back(std::make_shared<ProcessorCpp>(id));
  } else {
    std::cerr << mMessagePrefix + __FUNCTION__ + " Unsuported Processor type"
              << std::endl;
    return false;
  }
  const auto &newProc = mLocalProcs.back();

  auto inputDir = command.inputdirectory();
  std::vector<std::string> inputFiles(command.inputfiles().begin(),
                                      command.inputfiles().end());
  auto outputDir = command.outputdirectory();
  std::vector<std::string> outputFiles(command.outputfiles().begin(),
                                       command.outputfiles().end());
  auto runDir = command.runningdirectory();

  auto doc = command.documentation();

  newProc->setInputDirectory(inputDir);
  newProc->setInputFileNames(inputFiles);
  newProc->setOutputDirectory(outputDir);
  newProc->setOutputFileNames(outputFiles);
  newProc->setRunningDirectory(runDir);
  newProc->setDocumentation(doc);
  newProc->setVerbose(mVerbose); // Inherit verbosity

  if (registerProcessor(*newProc, src)) {
    if (forward) {
      sendRegisterMessage(newProc.get(), nullptr, src);
    }
  }
  return true;
}

bool TincProtocol::processRegisterDiskBuffer(void *any, al::Socket *src,
                                             bool forward) {

  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterDiskBuffer>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Register Disk Buffer message contains invalid payload"
              << std::endl;
    return false;
  }

  RegisterDiskBuffer command;
  details->UnpackTo(&command);
  auto id = command.id();

  if (mVerbose) {
    std::cout << mMessagePrefix + "Registering DiskBuffer " + id << std::endl;
  }

  for (auto &ps : mDiskBuffers) {
    if (ps->getId() == id) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": DiskBuffer " + id +
                         " already registered."
                  << std::endl;
      }
      // TODO verify consistency
      return true;
    }
  }
  auto path = command.path();
  auto baseFilename = path.filename();
  auto relPath = path.relativepath();

  std::string rootPath = path.rootpath();

  if (src) {
    rootPath = mapFromRemotePath(rootPath, src);
    if (mVerbose) {
      std::cout << "Mapping path for buffer: " << rootPath << std::endl;
    }
  }

  if (command.type() == DiskBufferType::JSON) {
    mLocalDBs.emplace_back(
        std::make_shared<DiskBufferJson>(id, baseFilename, relPath, rootPath));
  } else if (command.type() == DiskBufferType::NETCDF) {
    mLocalDBs.emplace_back(std::make_shared<DiskBufferNetCDFData>(
        id, baseFilename, relPath, rootPath));
  } else if (command.type() == DiskBufferType::IMAGE) {
    mLocalDBs.emplace_back(
        std::make_shared<DiskBufferImage>(id, baseFilename, relPath, rootPath));
  } else if (command.type() == DiskBufferType::TEXT) {
    mLocalDBs.emplace_back(
        std::make_shared<DiskBufferText>(id, baseFilename, relPath, rootPath));
  } /* else if (command.type() == DiskBufferType::BINARY) {
      mLocalDBs.emplace_back(
          std::make_shared<DiskBufferImage>(id, baseFilename, relPath,
  rootPath));
  } */
  else {

    std::cout << mMessagePrefix + __FUNCTION__ +
                     ": DiskBuffer type not supported."
              << std::endl;
    return false;
  }

  mLocalDBs.back()->setDocumentation(command.documentation());
  mLocalDBs.back()->setVerbose(mVerbose); // Inherit verbosity

  if (registerDiskBuffer(*mLocalDBs.back(), src)) {
    if (forward) {
      sendRegisterMessage(mLocalDBs.back().get(), nullptr, src);
    }
  }

  return true;
}

bool TincProtocol::processRegisterDataPool(void *any, al::Socket *src,
                                           bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterDataPool>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Register Data Pool message contains invalid payload"
              << std::endl;
    return false;
  }

  RegisterDataPool command;
  details->UnpackTo(&command);
  auto id = command.id();

  if (mVerbose) {
    std::cout << mMessagePrefix + " Registering DataPool " + id << std::endl;
  }

  for (auto &dp : mDataPools) {
    if (dp->getId() == id) {
      if (mVerbose) {
        std::cout << mMessagePrefix + __FUNCTION__ + ": DataPool " + id +
                         " already registered."
                  << std::endl;
      }
      return true;
    }
  }

  auto psId = command.parameterspaceid();
  auto cacheDir = command.cachedirectory();

  auto ps = getParameterSpace(psId);

  if (ps) {

    if (command.type() == DataPoolTypes::DATAPOOLTYPE_JSON) {
      mLocalDPs.emplace_back(std::make_shared<DataPoolJson>(id, *ps, cacheDir));
    } else if (command.type() == DataPoolTypes::DATAPOOLTYPE_NETCDF) {
      mLocalDPs.emplace_back(
          std::make_shared<DataPoolNetCDF>(id, *ps, cacheDir));
    } else if (command.type() == DataPoolTypes::DATAPOOLTYPE_USER) {

      std::cout << mMessagePrefix + __FUNCTION__ +
                       ": DataPool **USER** types not supported."
                << std::endl;
      return false;
    } else {

      std::cout << mMessagePrefix + __FUNCTION__ +
                       ": DataPool type not supported."
                << std::endl;
      return false;
    }
    mLocalDPs.back()->setDocumentation(command.documentation());
    mLocalDPs.back()->setVerbose(mVerbose); // Inherit verbosity

    if (registerDataPool(*mLocalDPs.back(), src)) {
      if (forward) {
        sendRegisterMessage(mLocalDPs.back().get(), nullptr, src);
      }
    }
    return true;
  } else {

    std::cout << mMessagePrefix + __FUNCTION__ +
                     ": ParameterSpace in DataPool not found. Datapool not "
                     "registered."
              << std::endl;
    return false;
  }

  return false;
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

  for (auto *dim : ps->getDimensions()) {
    sendRegisterMessage(dim, dst, src);
    sendConfigureParameterSpaceAddDimension(ps, dim, dst, src);
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
  registerProcMessage.set_documentation(p->getDocumentation());
  if (strcmp(typeid(*p).name(), typeid(ProcessorScript).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::DATASCRIPT);
  } else if (strcmp(typeid(*p).name(), typeid(ProcessorGraph).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::CHAIN);
  } else if (strcmp(typeid(*p).name(), typeid(ProcessorCpp).name()) == 0) {
    registerProcMessage.set_type(ProcessorType::CPP);
  }

  registerProcMessage.set_inputdirectory(p->getInputDirectory());
  for (auto &inFile : p->getInputFileNames()) {
    registerProcMessage.add_inputfiles(inFile);
  }
  registerProcMessage.set_outputdirectory(p->getOutputDirectory());
  for (auto &outFile : p->getOutputFileNames()) {
    registerProcMessage.add_outputfiles(outFile);
  }

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(registerProcMessage);
  msg.set_allocated_details(detailsAny);

  if (dynamic_cast<ProcessorGraph *>(p)) {
    for (auto &childProcessor :
         dynamic_cast<ProcessorGraph *>(p)->getProcessors()) {
      sendRegisterMessage(childProcessor, dst, src);
    }
  }

  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

std::string TincProtocol::mapFromRemotePath(std::string path, al::Socket *src) {
  for (auto &entry : mRootPathMap) {
    // TODO verify check for hostname is working
    //        std::cout << src->hostName() << " " << al::Socket::hostName() <<
    //        std::endl;
    bool isSameHost = false;
    if (mClientHostnames.find(src) != mClientHostnames.end()) {
      isSameHost = src->hostName() == mClientHostnames[src];
    }
    if (entry.first == "" && !isSameHost) {
      for (auto &mapEntry : entry.second) {
        if (al::File::isSamePath(mapEntry.second, path) ||
            mapEntry.second.find(path) == 0) {
          path = mapEntry.first + path.substr(mapEntry.second.size());
          goto path_mapped;
        }
      }
    }
  }
path_mapped:
  return path;
}

std::string TincProtocol::mapToRemotePath(std::string path, al::Socket *src) {
  for (auto &mapEntry : mRootPathMap) {
    bool isSameHost = false;
    if (mClientHostnames.find(src) != mClientHostnames.end()) {
      isSameHost = src->hostName() == mClientHostnames[src];
    }
    if (mapEntry.first == "" && !isSameHost) {
      for (auto pathMap : mapEntry.second) {
        if (al::File::isSamePath(pathMap.first, path)) {
          path = pathMap.second;
          goto path_mapped;
        }
      }
    }
  }
path_mapped:
  return path;
}

void TincProtocol::sendRegisterMessage(DiskBufferAbstract *db, al::Socket *dst,
                                       al::Socket *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REGISTER);
  msg.set_objecttype(ObjectType::DISK_BUFFER);

  RegisterDiskBuffer details;
  details.set_id(db->getId());
  details.set_documentation(db->getDocumentation());

  DiskBufferType type = DiskBufferType::BINARY;

  if (dynamic_cast<DiskBufferNetCDFData *>(db)) {
    type = DiskBufferType::NETCDF;
  } else if (dynamic_cast<DiskBufferImage *>(db)) {
    type = DiskBufferType::IMAGE;
  } else if (dynamic_cast<DiskBufferJson *>(db)) {
    type = DiskBufferType::JSON;
  } else if (dynamic_cast<DiskBufferText *>(db)) {
    type = DiskBufferType::TEXT;
  } else {
    // FIXME implement missing types
    type = DiskBufferType::BINARY;
  }
  details.set_type(type);

  tinc_protobuf::DistributedPath *path = new tinc_protobuf::DistributedPath;
  path->set_filename(db->getBaseFileName());
  path->set_relativepath(db->getRelativePath());
  std::string rootPath = mapToRemotePath(db->getRootPath(), dst);
  path->set_rootpath(rootPath);

  details.set_allocated_path(path);

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
  details.set_documentation(dp->getDocumentation());
  details.set_parameterspaceid(dp->getParameterSpace().getId());
  details.set_cachedirectory(dp->getCacheDirectory());
  details.set_type((tinc_protobuf::DataPoolTypes)dp->getType());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);

  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

bool TincProtocol::readConfigureMessage(int objectType, void *any,
                                        al::Socket *src, bool forward) {
  switch (objectType) {
  case ObjectType::PARAMETER:
    return processConfigureParameter(any, src, forward);
  case ObjectType::PROCESSOR:
    // FIXME implement configure processor
    //    return sendProcessors(src);
    break;
  case ObjectType::DISK_BUFFER:
    return processConfigureDiskBuffer(any, src, forward);
  case ObjectType::DATA_POOL:
    return processConfigureDataPool(any, src, forward);
    break;
  case ObjectType::PARAMETER_SPACE:
    return processConfigureParameterSpace(any, src, forward);
  default:
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Invalid ObjectType"
              << std::endl;
    break;
  }
  return false;
}

bool TincProtocol::processConfigureParameter(void *any, al::Socket *src,
                                             bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<ConfigureParameter>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }

  if (mVerbose) {
    std::cout << mMessagePrefix +
                     "Protocol received Configure Parameter message"
              << std::endl;
  }

  ConfigureParameter conf;
  details->UnpackTo(&conf);
  auto addr = conf.id();

  for (auto *dim : mParameterSpaceDimensions) {
    if (addr == dim->getFullAddress()) {
      return processConfigureParameterMessage(&conf, dim, src, forward);
    }
  }

  for (auto *ps : mParameterSpaces) {
    for (auto *dim : ps->getDimensions()) {
      if (addr == dim->getFullAddress()) {
        return processConfigureParameterMessage(&conf, dim, src, forward);
      }
    }
  }

  if (mVerbose) {
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Unable to find Parameter " +
                     addr
              << std::endl;
  }
  return false;
}

bool TincProtocol::processConfigureParameterSpace(void *any, al::Socket *src,
                                                  bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<ConfigureParameterSpace>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }

  if (mVerbose) {
    std::cout << mMessagePrefix +
                     "Protocol received Configure ParameterSpace message"
              << std::endl;
  }

  ConfigureParameterSpace conf;
  details->UnpackTo(&conf);
  auto id = conf.id();

  for (auto *ps : mParameterSpaces) {
    if (id == ps->getId()) {
      ParameterSpaceConfigureType command = conf.configurationkey();

      if (command == ParameterSpaceConfigureType::ADD_PARAMETER) {
        if (conf.configurationvalue().Is<ParameterValue>()) {
          ParameterValue val;
          conf.configurationvalue().UnpackTo(&val);
          auto addr = val.valuestring();

          for (auto *dim : ps->getDimensions()) {
            if (addr == dim->getFullAddress()) {
              if (mVerbose) {
                std::cout << mMessagePrefix + __FUNCTION__ + ": Parameter " +
                                 addr +
                                 " already registered in ParameterSpace " + id
                          << std::endl;
              }
              return true;
            }
          }

          for (auto &dim : mLocalPSDs) {
            if (addr == dim->getFullAddress()) {
              ps->registerDimension(dim, src);
              if (mVerbose) {
                std::cout << mMessagePrefix + "Registered Parameter " + addr +
                                 " to ParameterSpace " + id
                          << std::endl;
              }
              // FIXME propagate to clients
              return true;
            }
          }

          std::cerr << mMessagePrefix + __FUNCTION__ +
                           ": Unable to find Parameter " + addr
                    << std::endl;
          return false;
        }
      } else if (command == ParameterSpaceConfigureType::REMOVE_PARAMETER) {
        if (conf.configurationvalue().Is<ParameterValue>()) {
          ParameterValue val;
          conf.configurationvalue().UnpackTo(&val);
          auto addr = val.valuestring();

          for (auto *dim : ps->getDimensions()) {
            if (addr == dim->getFullAddress()) {
              ps->removeDimension(dim->getName(), dim->getGroup(), true, src);
              if (mVerbose) {
                std::cout << mMessagePrefix + __FUNCTION__ +
                                 ": Removing Parameter " + addr +
                                 " from ParameterSpace " + id
                          << std::endl;
              }
              // FIXME propagate to clients
              return true;
            }
          }

          std::cerr << mMessagePrefix + __FUNCTION__ +
                           "Unable to find Parameter " + addr
                    << std::endl;
          return false;
        }
      } else if (command == ParameterSpaceConfigureType::CURRENT_TEMPLATE) {
        if (conf.configurationvalue().Is<ParameterValue>()) {
          ParameterValue val;
          conf.configurationvalue().UnpackTo(&val);
          auto curTempl = val.valuestring();

          ps->setCurrentPathTemplate(curTempl, src);
          if (forward) {
            sendConfigureMessage(ps, nullptr, src);
          }
          return true;
        }
      } else if (command == ParameterSpaceConfigureType::ROOT_PATH) {
        if (conf.configurationvalue().Is<ParameterValue>()) {
          ParameterValue val;
          conf.configurationvalue().UnpackTo(&val);
          auto curTempl = val.valuestring();

          ps->setRootPath(curTempl, src);
          if (forward) {
            sendConfigureMessage(ps, nullptr, src);
          }
          return true;
        }
      } else if (command == ParameterSpaceConfigureType::CACHE_PATH) {
        if (conf.configurationvalue().Is<tinc_protobuf::DistributedPath>()) {
          tinc_protobuf::DistributedPath path;
          conf.configurationvalue().UnpackTo(&path);

          ps->enableCache(path.relativepath());
          if (forward) {
            sendConfigureMessage(ps, nullptr, src);
          }
          return true;
        }
      } else if (command == ParameterSpaceConfigureType::PS_DOCUMENTATION) {
        ps->setDocumentation(conf.configurationvalue().value(), src);
        if (forward) {
          sendConfigureMessage(ps, nullptr, src);
        }
        return true;
      }

      std::cerr << mMessagePrefix + __FUNCTION__ +
                       ": Unrecognized configure message for ParameterSpace " +
                       id
                << std::endl;
      return false;
    }
  }

  std::cerr << mMessagePrefix + __FUNCTION__ +
                   ": Unable to find ParameterSpace "
            << id << std::endl;
  return false;
}

bool TincProtocol::processConfigureProcessor(void *any, al::Socket *src,
                                             bool forward) {
  // FIXME implement
  if (mVerbose) {
    std::cout
        << mMessagePrefix +
               "Protocol received Configure Processor message [not implemented]"
        << std::endl;
  }
  return true;
}

bool TincProtocol::processConfigureDiskBuffer(void *any, al::Socket *src,
                                              bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<ConfigureDiskBuffer>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }

  if (mVerbose) {
    std::cout << mMessagePrefix +
                     "Protocol received Configure DiskBuffer message "
              << std::endl;
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
          if (!db->loadData(file.valuestring(), false)) {
            std::cerr << mMessagePrefix + __FUNCTION__ +
                             ": Error updating DiskBuffer " + id +
                             " from file " + file.valuestring()
                      << std::endl;
            return false;
          }
          std::cout << mMessagePrefix + "DiskBuffer " + id +
                           " successfully loaded " + file.valuestring()
                    << std::endl;
          if (forward) {
            sendConfigureMessage(db, nullptr, src);
          }
          return true;
        }
      }
      std::cerr << mMessagePrefix + __FUNCTION__ +
                       ": Error configuring DiskBuffer " + id
                << std::endl;
      return false;
    }
  }

  std::cerr << mMessagePrefix + __FUNCTION__ + ": Unable to find DiskBuffer " +
                   id
            << std::endl;
  return false;
}

bool TincProtocol::processConfigureDataPool(void *any, al::Socket *src,
                                            bool forward) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<ConfigureDataPool>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Configure message contains invalid payload"
              << std::endl;
    return false;
  }

  if (mVerbose) {
    std::cout << mMessagePrefix +
                     "Protocol received Configure DataPool message "
              << std::endl;
  }

  ConfigureDataPool conf;
  details->UnpackTo(&conf);
  auto id = conf.id();
  for (auto *dp : mDataPools) {
    if (id == dp->getId()) {
      DataPoolConfigureType command = conf.configurationkey();

      if (command == DataPoolConfigureType::SLICE_CACHE_DIR) {
        if (conf.configurationvalue().Is<ParameterValue>()) {
          ParameterValue file;
          conf.configurationvalue().UnpackTo(&file);
          dp->setCacheDirectory(file.valuestring(), src);

          if (forward) {
            sendConfigureMessage(dp, nullptr, src);
          }
          return true;
        }
      } else if (command == DataPoolConfigureType::DATA_FILES) {
        if (conf.configurationvalue().Is<DataPoolDataFiles>()) {
          DataPoolDataFiles files;
          conf.configurationvalue().UnpackTo(&files);
          if (files.filename_size() != files.dimension_size()) {
            std::cerr << mMessagePrefix + __FUNCTION__ +
                             ": Error configuring DataPool DATA_FILES size "
                             "mismatch " +
                             id
                      << std::endl;
          }
          for (int i = 0; i < files.filename_size(); i++) {
            dp->clearRegisteredFiles();
            dp->registerDataFile(files.filename(i), files.dimension(i));
          }

          if (forward) {
            sendConfigureMessage(dp, nullptr, src);
          }
          return true;
        }
      }
      std::cerr << mMessagePrefix + __FUNCTION__ +
                       ": Error configuring DataPool " + id
                << std::endl;
      return false;
    }
  }

  std::cerr << mMessagePrefix + __FUNCTION__ + ": Unable to find DataPool " + id
            << std::endl;
  return false;
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
  auto msgs = createConfigureParameterSpaceMessage(ps);
  for (auto &msg : msgs) {
    if (src) {
      sendTincMessage(&msg, dst, src->valueSource());
    } else {
      sendTincMessage(&msg, dst);
    }
  }
  //  for (auto *dim : ps->getDimensions()) {
  //    sendConfigureMessage(dim, dst, src);
  //  }
}

void TincProtocol::sendConfigureMessage(Processor *p, al::Socket *dst,
                                        al::Socket *src) {
  // if processor is asynchronous
  if (strcmp(typeid(*p).name(), typeid(ProcessorAsyncWrapper).name()) == 0) {
    p = dynamic_cast<ProcessorAsyncWrapper *>(p)->processor();
  }

  for (auto &config : p->configuration) {
    TincMessage msg;
    msg.set_messagetype(MessageType::CONFIGURE);
    msg.set_objecttype(ObjectType::PROCESSOR);
    ConfigureProcessor confMessage;
    confMessage.set_id(p->getId());
    confMessage.set_configurationkey(config.first);
    google::protobuf::Any *configValue = confMessage.configurationvalue().New();
    ParameterValue val;
    // TODO ML support all types
    if (config.second.type() == al::VariantType::VARIANT_DOUBLE) {
      val.set_valuedouble(config.second.get<double>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_DOUBLE));
    } else if (config.second.type() == al::VariantType::VARIANT_FLOAT) {
      val.set_valuedouble(config.second.get<float>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
    } else if (config.second.type() == al::VariantType::VARIANT_INT64) {
      val.set_valueint64(config.second.get<int64_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT64));
    } else if (config.second.type() == al::VariantType::VARIANT_INT32) {
      val.set_valueint32(config.second.get<int32_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
    } else if (config.second.type() == al::VariantType::VARIANT_INT16) {
      val.set_valueint32(config.second.get<int16_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT16));
    } else if (config.second.type() == al::VariantType::VARIANT_INT8) {
      val.set_valueint32(config.second.get<int8_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT8));
    } else if (config.second.type() == al::VariantType::VARIANT_UINT64) {
      val.set_valueuint64(config.second.get<uint64_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_UINT64));
    } else if (config.second.type() == al::VariantType::VARIANT_UINT32) {
      val.set_valueuint32(config.second.get<uint32_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_UINT32));
    } else if (config.second.type() == al::VariantType::VARIANT_UINT16) {
      val.set_valueuint32(config.second.get<uint16_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_UINT16));
    } else if (config.second.type() == al::VariantType::VARIANT_UINT8) {
      val.set_valueuint32(config.second.get<uint8_t>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_UINT8));
    } else if (config.second.type() == al::VariantType::VARIANT_STRING) {
      val.set_valuestring(config.second.get<std::string>());
      val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
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
    for (auto &childProcessor :
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
  auto msg = createConfigureDiskBufferMessage(p);
  if (src) {
    sendTincMessage(&msg, dst, src->valueSource());
  } else {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendConfigureMessage(DataPool *p, al::Socket *dst,
                                        al::Socket *src) {
  auto msgs = createConfigureDataPoolMessage(p);
  if (src) {
    for (auto &msg : msgs) {
      sendTincMessage(&msg, dst, src->valueSource());
    }
  } else {
    for (auto &msg : msgs) {
      sendTincMessage(&msg, dst);
    }
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

bool TincProtocol::readRemoveMessage(int objectType, void *any,
                                     al::Socket *src) {
  switch (objectType) {
  case ObjectType::PARAMETER:
    return processRemoveParameter(any, src);
  case ObjectType::PROCESSOR:
    // return processRemoveProcessor(any, src);
  case ObjectType::DISK_BUFFER:
    // return processRemoveDiskBuffer(any, src);
  case ObjectType::DATA_POOL:
    // return processRemoveDataPool(any, src);
  case ObjectType::PARAMETER_SPACE:
    // return processRemoveParameterSpace(any, src);
  default:
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Invalid ObjectType"
              << std::endl;
    break;
  }
  return false;
}

bool TincProtocol::processRemoveParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RemoveParameter>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Remove message contains invalid payload"
              << std::endl;
    return false;
  }

  RemoveParameter command;
  details->UnpackTo(&command);
  auto name = command.id();
  auto group = command.group();

  for (auto *dim : mParameterSpaceDimensions) {
    if (dim->getName() == name && dim->getGroup() == group) {
      removeParameter(name, group, true, src);
      return true;
    }
  }

  std::cerr << mMessagePrefix + __FUNCTION__ + ": Unable to find Parameter " +
                   name + " (Group: " + group + ") to remove."
            << std::endl;
  return false;
}

void TincProtocol::sendRemoveMessage(ParameterSpaceDimension *dim,
                                     al::Socket *dst, al::Socket *src) {
  TincMessage msg;
  msg.set_messagetype(MessageType::REMOVE);
  msg.set_objecttype(ObjectType::PARAMETER);

  google::protobuf::Any *details = msg.details().New();

  RemoveParameter removeDetails;
  removeDetails.set_id(dim->getName());
  removeDetails.set_group(dim->getGroup());
  details->PackFrom(removeDetails);

  msg.set_allocated_details(details);

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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_BOOL));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT32));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_INT64));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_STRING));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
                 static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
                 static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
                 static_cast<uint16_t>(al::VariantType::VARIANT_FLOAT));
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
  val.set_nctype(static_cast<uint16_t>(al::VariantType::VARIANT_VECTOR_OFFSET) +
                 static_cast<uint16_t>(al::VariantType::VARIANT_DOUBLE));
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

bool TincProtocol::readPingMessage(int objectType, void *any, al::Socket *src) {
  if (objectType != ObjectType::GLOBAL) {
    std::cerr << mMessagePrefix + __FUNCTION__ + " ERROR: PING expects GLOBAL."
              << std::endl;
    return false;
  }
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (details->Is<ParameterValue>()) {
    ParameterValue pingCode;
    details->UnpackTo(&pingCode);
    //    std::cout << " Got PING " << pingCode.valueuint64() << std::endl;
    auto msg = createPongMessage(pingCode.valueuint64());
    sendTincMessage(&msg, src);
    return true;
  } else {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Ping message contains invalid payload"
              << std::endl;
    return false;
  }
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
    return processCommandDiskBuffer(any, src);
    break;
  case ObjectType::DATA_POOL:
    return processCommandDataPool(any, src);
  case ObjectType::PARAMETER_SPACE:
    return processCommandParameterSpace(any, src);
  default:
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Invalid ObjectType"
              << std::endl;
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
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Command message contains invalid payload"
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
      for (auto *dim : ps->getDimensions()) {
        if (dim->getFullAddress() == id) {
          if (al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(
                  dim->getParameterMeta())) {
            elements = p->getElements();
            break;
          }
        }
      }
    }
    for (auto &dim : mParameterSpaceDimensions) {
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
    for (auto &ps : mParameterSpaces) {
      if (ps->getId() == psId) {
        auto curDir = ps->getCurrentRelativeRunPath();

        if (mVerbose) {
          std::cout << mMessagePrefix + std::to_string(commandNumber) +
                           ":****: " + curDir
                    << std::endl;
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
    for (auto &ps : mParameterSpaces) {
      if (ps->getId() == psId) {
        auto rootPath = ps->getRootPath();

        if (mVerbose) {
          std::cout << mMessagePrefix + std::to_string(commandNumber) +
                           ":****: " + rootPath
                    << std::endl;
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

bool TincProtocol::processCommandDiskBuffer(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Command message contains invalid payload"
              << std::endl;
    return false;
  }
  Command incomingCommand;
  details->UnpackTo(&incomingCommand);
  //  uint64_t commandNumber = incomingCommand.message_id();
  auto dblId = incomingCommand.id().id();

  if (incomingCommand.details().Is<DataPoolCommandSlice>()) {
  }
  return true;
}

bool TincProtocol::processCommandDataPool(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << mMessagePrefix + __FUNCTION__ +
                     ": Command message contains invalid payload"
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

    for (auto &dp : mDataPools) {
      if (dp->getId() == datapoolId) {
        auto sliceName = dp->createDataSlice(field, dims);

        if (mVerbose) {
          std::cout << mMessagePrefix + "DataPool slice command reply: " +
                           std::to_string(commandNumber) + "  " + sliceName
                    << std::endl;
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

    for (auto &dp : mDataPools) {
      if (dp->getId() == datapoolId) {
        auto filenames = dp->getCurrentFiles();

        if (mVerbose) {
          std::cout << mMessagePrefix + " DataPool command current files: " +
                           std::to_string(commandNumber) + " replying"
                    << std::endl;
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
        DataPoolCommandCurrentFilesReply reply;
        for (auto &f : filenames) {
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
    std::cerr << mMessagePrefix + __FUNCTION__ + ": Error serializing message"
              << std::endl;
  }

  if (mVerbose) {
    //    std::cout << __FUNCTION__ << ": Sending bytes " << size << std::endl;
    std::cout << mMessagePrefix + __FUNCTION__ + ": Sending " +
                     MessageType_Name(((TincMessage *)&msg)->messagetype()) +
                     " " +
                     ObjectType_Name(((TincMessage *)&msg)->objecttype()) +
                     " bytes " + std::to_string(size)
              << std::endl;
  }
  {
    std::unique_lock<std::mutex> lk(mSendLock);
    auto bytes = dst->send(buffer, size + sizeof(size_t));
    if (bytes != size + sizeof(size_t)) {
      if (bytes == SIZE_MAX) {
        std::cerr << mMessagePrefix + __FUNCTION__ + " Connection error."
                  << std::endl;
      } else {
        std::cerr << mMessagePrefix + __FUNCTION__ + ": Error sending " +
                         std::to_string(bytes) + " expected " +
                         std::to_string(size + sizeof(size_t)) + " (" +
                         strerror(errno) + ")"
                  << std::endl;
      }
      free(buffer);

#ifdef AL_WINDOWS
      int error = WSAGetLastError();
      std::cerr << mMessagePrefix + "Winsock error: " + std::to_string(error)
                << std::endl;
#endif
      return false;
    }
  }

  free(buffer);
  return true;
}
