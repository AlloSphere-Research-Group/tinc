#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"
#include "tinc/ProcessorAsync.hpp"
#include "tinc/TincClient.hpp"

#include <iostream>
#include <memory>

#include "tinc_protocol.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace tinc;

// TODO namespace these functions to avoid potential clashes
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
    // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    //    configValue->PackFrom(p->get());
    assert(1 == 0); // Implement!
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
    // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    //    configValue->PackFrom(p->get());
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    //    configValue->PackFrom(p->get());
    assert(1 == 0); // Implement!
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
    // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
    assert(1 == 0); // Implement!
  } else {
    assert(1 == 0); // Implement!
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

TincMessage createConfigureParameterFromDim(ParameterSpaceDimension *dim) {
  TincMessage msg;
  msg.set_messagetype(CONFIGURE);
  msg.set_objecttype(PARAMETER);
  ConfigureParameter confMessage = createParameterSpaceIdValuesMessage(dim);

  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(confMessage);
  msg.set_allocated_details(detailsAny);
  return msg;
}

std::vector<TincMessage> createParameterFloatInfoMessage(al::Parameter *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
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

    messages.push_back(msg);
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

    messages.push_back(msg);
  }
  return messages;
}

std::vector<TincMessage>
createParameterIntInfoMessage(al::ParameterInt *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
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
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

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
  return messages;
}

std::vector<TincMessage>
createParameterStringInfoMessage(al::ParameterString *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

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
createParameterChoiceInfoMessage(al::ParameterChoice *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

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
createParameterColorInfoMessage(al::ParameterColor *param) {
  std::vector<TincMessage> messages;
  {
    TincMessage msg;
    msg.set_messagetype(CONFIGURE);
    msg.set_objecttype(PARAMETER);

    google::protobuf::Any *details = msg.details().New();
    ConfigureParameter confMessage;
    createParameterValueMessage(param, confMessage);
    details->PackFrom(confMessage);
    msg.set_allocated_details(details);
    messages.push_back(msg);
  }
  return messages;
}

TincMessage createRegisterParameterMessage(al::ParameterMeta *param) {
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
    // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    // details.set_datatype(PARAMETER_POSED);
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
    // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    // details.set_datatype(PARAMETER_VEC3F);
    // al::Vec3f defaultValue = p->getDefault();
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    // details.set_datatype(PARAMETER_VEC4F);
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
    // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
    // details.set_datatype(PARAMETER_TRIGGER);
    assert(1 == 0); // Implement!
  } else {
    assert(1 == 0); // Implement!
  }
  auto details_any = msg.details().New();
  details_any->PackFrom(details);
  msg.set_allocated_details(details_any);
  return msg;
}

std::vector<TincMessage> createParameterInfoMessage(al::ParameterMeta *param) {
  if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
    al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
    return createParameterFloatInfoMessage(p);
  } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0) {
    al::Parameter *p = dynamic_cast<al::Parameter *>(param);
    return createParameterFloatInfoMessage(p);
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterInt).name()) ==
             0) {
    al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
    return createParameterIntInfoMessage(p);
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterString).name()) == 0) { //
    al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
    return createParameterStringInfoMessage(p);
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterPose).name()) ==
             0) { // al::ParameterPose
    // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterMenu).name()) ==
             0) { // al::ParameterMenu
    // al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(),
                    typeid(al::ParameterChoice).name()) ==
             0) { // al::ParameterChoice
    al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
    return createParameterChoiceInfoMessage(p);
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec3).name()) ==
             0) { // al::ParameterVec3
    // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterVec4).name()) ==
             0) { // al::ParameterVec4
    // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
    assert(1 == 0); // Implement!
  } else if (strcmp(typeid(*param).name(), typeid(al::ParameterColor).name()) ==
             0) { // al::ParameterColor
    al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
    return createParameterColorInfoMessage(p);
  } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
             0) { // Trigger
    // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
    assert(1 == 0); // Implement!
  } else {
    assert(1 == 0); // Implement!
  }
  return {};
}

TincMessage createRegisterParameterSpaceMessage(ParameterSpace *ps) {
  TincMessage msg;
  msg.set_messagetype(REGISTER);
  msg.set_objecttype(PARAMETER_SPACE);
  RegisterParameterSpace details;
  details.set_id(ps->getId());
  google::protobuf::Any *detailsAny = msg.details().New();
  detailsAny->PackFrom(details);
  msg.set_allocated_details(detailsAny);
  return msg;
}

TincMessage createConfigureDataPoolMessage(DataPool *p) {
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
  return msg;
}

//// ------------------------------------------------------
void TincProtocol::registerParameter(al::ParameterMeta &pmeta) {
  bool registered = false;
  for (auto *p : mParameters) {
    // FIXME reevaluate if name/group string comparison is needed
    if (p == &pmeta || (p->getName() == pmeta.getName() &&
                        p->getGroup() == pmeta.getGroup())) {
      registered = true;
      break;
    }
  }
  if (!registered) {
    mParameters.push_back(&pmeta);
    al::ParameterMeta *param = &pmeta;
    // TODO ensure parameter has not been registered (directly or through a
    // parameter space)

    // FIXME is strcmp really necessary?
    if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) == 0) {
      al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
      p->registerChangeCallback([&, p](float value, al::ValueSource *src) {
        sendParameterFloatValue(value, p->getFullAddress(), src);
      });
    } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) ==
               0) {
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
                      typeid(al::ParameterString).name()) ==
               0) { // al::Parameter
      // al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
      assert(1 == 0); // Implement!
    } else if (strcmp(typeid(*param).name(),
                      typeid(al::ParameterPose).name()) ==
               0) { // al::ParameterPose
      // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
      assert(1 == 0); // Implement!
    } else if (strcmp(typeid(*param).name(),
                      typeid(al::ParameterMenu).name()) ==
               0) { // al::ParameterMenu
      // al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
      assert(1 == 0); // IMplement!
    } else if (strcmp(typeid(*param).name(),
                      typeid(al::ParameterChoice).name()) ==
               0) { // al::ParameterChoice
      al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
      p->registerChangeCallback([&, p](uint64_t value, al::ValueSource *src) {
        sendParameterUint64Value(value, p->getFullAddress(), src);
      });
    } else if (strcmp(typeid(*param).name(),
                      typeid(al::ParameterVec3).name()) ==
               0) { // al::ParameterVec3
      // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
      assert(1 == 0); // Implement!
    } else if (strcmp(typeid(*param).name(),
                      typeid(al::ParameterVec4).name()) ==
               0) { // al::ParameterVec4
      // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
      assert(1 == 0); // Implement!
    } else if (strcmp(typeid(*param).name(),
                      typeid(al::ParameterColor).name()) ==
               0) { // al::ParameterColor
      al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
      p->registerChangeCallback([&, p](al::Color value, al::ValueSource *src) {
        sendParameterColorValue(value, p->getFullAddress(), src);
      });
    } else if (strcmp(typeid(*param).name(), typeid(al::Trigger).name()) ==
               0) { // Trigger
      // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
      assert(1 == 0); // Implement!
    } else {
      assert(1 == 0); // Implement!
    }
  } else {
    if (mVerbose) {
      std::cout << "Parameter: " << pmeta.getName()
                << " (Group: " << pmeta.getGroup() << ") already registered."
                << std::endl;
    }
  }
}

void TincProtocol::registerParameterSpace(ParameterSpace &ps) {
  bool registered = false;
  for (auto *p : mParameterSpaces) {
    if (p == &ps || p->getId() == ps.getId()) {
      registered = true;
      break;
    }
  }
  for (auto dim : ps.getDimensions()) {
    registerParameterSpaceDimension(*dim);
  }
  if (!registered) {
    mParameterSpaces.push_back(&ps);
    // FIXME re-check callback function. something doesn't look right
    ps.onDimensionRegister = [&](ParameterSpaceDimension *changedDimension,
                                 ParameterSpace *ps) {
      for (auto dim : ps->getDimensions()) {
        if (dim->getName() == changedDimension->getName()) {
          auto msg = createConfigureParameterFromDim(changedDimension);
          sendTincMessage(&msg);
          if (dim.get() != changedDimension) {
            registerParameterSpaceDimension(*changedDimension);
          }
        }
        auto msg = createRegisterParameterSpaceMessage(ps);
        sendTincMessage(&msg);

        for (auto dim : ps->getDimensions()) {
          //            create
          TincMessage msg = createRegisterParameterMessage(&dim->parameter());
          sendTincMessage(&msg);

          auto infoMessages = createParameterInfoMessage(&dim->parameter());
          for (auto &msg : infoMessages) {
            sendTincMessage(&msg);
          }
          msg = createConfigureParameterFromDim(dim.get());
          sendTincMessage(&msg);
        }
      }
    };
  } else {
    if (mVerbose) {
      std::cout << "Processor: " << ps.getId() << " already registered."
                << std::endl;
    }
  }
}

void TincProtocol::registerParameterSpaceDimension(
    ParameterSpaceDimension &psd) {
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

    psd.onDimensionMetadataChange =
        [&](ParameterSpaceDimension *changedDimension) {
          registerParameterSpaceDimension(*changedDimension);

          TincMessage msg =
              createRegisterParameterMessage(&changedDimension->parameter());
          sendTincMessage(&msg);

          auto infoMessages =
              createParameterInfoMessage(&changedDimension->parameter());
          for (auto &msg : infoMessages) {
            sendTincMessage(&msg);
          }
          msg = createConfigureParameterFromDim(changedDimension);
          sendTincMessage(&msg);
        };
  } else {
    if (mVerbose) {
      std::cout << "ParameterSpaceDimension: " << psd.getFullAddress()
                << " already registered." << std::endl;
    }
  }
}

void TincProtocol::registerProcessor(Processor &processor) {
  bool registered = false;
  for (auto *p : mProcessors) {
    if (p == &processor || p->getId() == processor.getId()) {
      registered = true;
      break;
    }
  }
  if (!registered) {
    mProcessors.push_back(&processor);
    // FIXME we should register parameters registered with processors.
    //    for (auto *param: processor.parameters()) {
    //    }
  } else {
    if (mVerbose) {
      std::cout << "Processor: " << processor.getId() << " already registered."
                << std::endl;
    }
  }
}

void TincProtocol::registerDiskBuffer(AbstractDiskBuffer &db) {
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
      std::cout << "DiskBuffer: " << db.getId() << " already registered."
                << std::endl;
    }
  }
}

void TincProtocol::registerDataPool(DataPool &dp) {
  bool registered = false;
  for (auto *p : mDataPools) {
    if (p == &dp || p->getId() == dp.getId()) {
      registered = true;
    }
  }
  if (!registered) {
    registerParameterSpace(dp.getParameterSpace());
    mDataPools.push_back(&dp);
    dp.modified = [&]() {
      auto msg = createConfigureDataPoolMessage(&dp);
      sendTincMessage(&msg);
    };
  } else {
    // FIXME replace entry in mDataPools wiht this one if it is not the same
    // instance
    if (mVerbose) {
      std::cout << "DiskBuffer: " << dp.getId() << " already registered."
                << std::endl;
    }
  }
}

void TincProtocol::registerParameterServer(al::ParameterServer &pserver) {
  bool registered = false;
  for (auto *p : mParameterServers) {
    if (p == &pserver) {
      registered = true;
      break;
    }
  }
  if (!registered) {
    mParameterServers.push_back(&pserver);
  } else {
    if (mVerbose) {
      std::cout << "ParameterServer already registered." << std::endl;
    }
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

TincProtocol &TincProtocol::operator<<(AbstractDiskBuffer &db) {
  registerDiskBuffer(db);
  return *this;
}

TincProtocol &TincProtocol::operator<<(DataPool &dp) {
  registerDataPool(dp);
  return *this;
}

TincProtocol &TincProtocol::operator<<(al::ParameterServer &pserver) {
  registerParameterServer(pserver);
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

void TincProtocol::sendParameters(al::Socket *dst) {
  std::cout << __FUNCTION__ << std::endl;
  for (auto *p : mParameters) {
    sendRequestResponse(p, dst);
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
  auto msg = createConfigureDataPoolMessage(p);
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

void TincProtocol::sendRegisterParameterSpaceMessage(ParameterSpace *ps,
                                                     al::Socket *dst) {

  auto msg = createRegisterParameterSpaceMessage(ps);

  sendProtobufMessage(&msg, dst);
  for (auto dim : ps->getDimensions()) {
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
  TincMessage msg = createConfigureParameterFromDim(dim);

  sendProtobufMessage(&msg, dst);
}

void TincProtocol::sendRegisterParameterMessage(al::ParameterMeta *param,
                                                al::Socket *dst) {
  TincMessage msg = createRegisterParameterMessage(param);
  sendTincMessage(&msg, dst);

  auto infoMessages = createParameterInfoMessage(param);
  for (auto &msg : infoMessages) {
    sendTincMessage(&msg, dst);
  }
}

void TincProtocol::sendRequestResponse(al::ParameterMeta *param,
                                       al::Socket *dst) {
  TincMessage msg = createRegisterParameterMessage(param);
  sendProtobufMessage(&msg, dst);

  auto infoMessages = createParameterInfoMessage(param);
  for (auto &msg : infoMessages) {
    sendProtobufMessage(&msg, dst);
  }
}

bool TincProtocol::runRegister(int objectType, void *any, al::Socket *src) {
  switch (objectType) {
  case PARAMETER:
    return processRegisterParameter(any, src);
  case PROCESSOR:
  //    return sendProcessors(src);
  case DISK_BUFFER:
    return processRegisterDiskBuffer(any, src);
  case DATA_POOL:
  //    return sendDataPools(src);
  case PARAMETER_SPACE:
    //    return sendParameterSpace(src);
    break;
  }
  return false;
}

bool TincProtocol::processRegisterParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<RegisterParameter>()) {
    std::cerr << "Error: Invalid payload for RegisterParameter" << std::endl;
    return false;
  }
  RegisterParameter command;
  details->UnpackTo(&command);
  auto id = command.id();
  auto group = command.group();
  auto def = command.defaultvalue();
  auto datatype = command.datatype();
  for (auto *param : mParameters) {
    if (param->getName() == id && param->getGroup() == group) {
      // TODO should we replace the existing parameter?
      std::cout << "Parameter " << id << " already registered" << std::endl;
      return true;
    }
  }
  // ParameterSpaceDimension *param = nullptr;
  al::ParameterInt *param = nullptr;
  switch (datatype) {
  case ParameterDataType::PARAMETER_FLOAT:
    // param = new ParameterSpaceDimension(id, group);
    // param->parameter().setDefault(def.valuefloat());
    // registerParameterSpaceDimension(*param);
    // sendRegisterParameterSpaceDimensionMessage(param, src);
    break;
  case ParameterDataType::PARAMETER_STRING:
    // FIXME implement
    //    param = new al::ParameterString(id, group, def.valuestring());
    //    registerParameter(*param);
    //    sendRegisterParameterMessage(param, src);
    break;
  case ParameterDataType::PARAMETER_INT32:
    param = new al::ParameterInt(id, group, def.valueint32());
    std::cout << " Parameter: " << id << std::endl;
    registerParameter(*param);
    std::cout << "between register ftn" << std::endl;
    sendRegisterParameterMessage(param, src);
    break;
  case ParameterDataType::PARAMETER_CHOICE:
    // FIXME implement
    //    param = new al::ParameterChoice(id, group, def.valueuint64());
    //    registerParameter(*param);
    //    sendRegisterParameterMessage(param, src);
    break;
  case ParameterDataType::PARAMETER_BOOL:
    // FIXME implement
    //    param = new al::ParameterBool(id, group, def.valuefloat());
    //    registerParameter(*param);
    //    sendRegisterParameterMessage(param, src);
    break;
  case ParameterDataType::PARAMETER_VEC3F:
  case ParameterDataType::PARAMETER_VEC4F:
  case ParameterDataType::PARAMETER_COLORF:
  case ParameterDataType::PARAMETER_POSED:
  case ParameterDataType::PARAMETER_MENU:
  case ParameterDataType::PARAMETER_TRIGGER:
  default: // ParameterDataType_INT_MIN_SENTINEL_DO_NOT_USE_ &
           // ParameterDataType_INT_MAX_SENTINEL_DO_NOT_USE_
    break;
  }
  if (param) {
    return true;
  }
  return false;
}

bool TincProtocol::processRegisterParameterSpace(al::Message &message,
                                                 al::Socket *src) {
  return true;
}

bool TincProtocol::processRegisterProcessor(al::Message &message,
                                            al::Socket *src) {
  return true;
}

bool TincProtocol::processRegisterDataPool(al::Message &message,
                                           al::Socket *src) {
  return true;
}

bool TincProtocol::processRegisterDiskBuffer(void *any, al::Socket *src) {
  return true;
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

bool processParameterConfigure(ConfigureParameter &conf,
                               al::ParameterMeta *param, al::Socket *src) {
  ParameterConfigureType command = conf.configurationkey();
  if (command == ParameterConfigureType::VALUE) {
    ParameterValue v;
    if (conf.configurationvalue().Is<ParameterValue>()) {
      conf.configurationvalue().UnpackTo(&v);
      if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) ==
          0) {
        al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
        p->set(v.valuefloat());
      } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) ==
                 0) {
        al::Parameter *p = dynamic_cast<al::Parameter *>(param);
        p->set(v.valuefloat());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterInt).name()) == 0) {
        al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
        std::cout << " value change from " << src->valueSource()->port
                  << std::endl;
        std::cout << " value: " << v.valueint32() << std::endl;
        p->set(v.valueint32(), src->valueSource());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterString).name()) == 0) { //
        al::ParameterString *p = dynamic_cast<al::ParameterString *>(param);
        p->set(v.valuestring());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterPose).name()) ==
                 0) { // al::ParameterPose
        // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterMenu).name()) ==
                 0) { // al::ParameterMenu
        // al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterChoice).name()) ==
                 0) { // al::ParameterChoice
        al::ParameterChoice *p = dynamic_cast<al::ParameterChoice *>(param);
        p->set(v.valueuint64());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterVec3).name()) ==
                 0) { // al::ParameterVec3
        // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterVec4).name()) ==
                 0) { // al::ParameterVec4
        // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterColor).name()) ==
                 0) { // al::ParameterColor
        al::ParameterColor *p = dynamic_cast<al::ParameterColor *>(param);
        if (v.valuelist_size() != 4) {
          std::cerr << "Unexpected number of components for ParameterColor"
                    << std::endl;
          return false;
        }
        al::Color value(
            v.valuelist(0).valuefloat(), v.valuelist(1).valuefloat(),
            v.valuelist(2).valuefloat(), v.valuelist(3).valuefloat());
        p->set(value);

      } else if (strcmp(typeid(*param).name(),
                        typeid(al::Trigger).name()) == 0) { // Trigger
        // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
        assert(1 == 0); // Implement!
      } else {
        assert(1 == 0); // Implement!
      }
    }
  } else if (command == ParameterConfigureType::MIN) {
    ParameterValue v;
    if (conf.configurationvalue().Is<ParameterValue>()) {
      conf.configurationvalue().UnpackTo(&v);
      if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) ==
          0) {
        al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
        p->min(v.valuefloat());
      } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) ==
                 0) {
        al::Parameter *p = dynamic_cast<al::Parameter *>(param);
        p->min(v.valuefloat());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterInt).name()) == 0) {
        al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
        std::cout << " min change from " << src->valueSource()->port
                  << std::endl;
        p->min(v.valueint32());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterString).name()) == 0) { //
        //                al::ParameterString *p =
        //                dynamic_cast<al::ParameterString *>(param);
        //                p->set(v.valuestring());
        std::cout << "Unexpected minimum value configure for ParameterString"
                  << std::endl;
        return true;
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterPose).name()) ==
                 0) { // al::ParameterPose
        // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterMenu).name()) ==
                 0) { // al::ParameterMenu
        // al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterChoice).name()) ==
                 0) { // al::ParameterChoice
        std::cout << "Unexpected minimum value configure for ParameterChoice"
                  << std::endl;
        return true;
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterVec3).name()) ==
                 0) { // al::ParameterVec3
        // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterVec4).name()) ==
                 0) { // al::ParameterVec4
        // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterColor).name()) ==
                 0) { // al::ParameterColor
        std::cout << "Unexpected minimum value configure for ParameterColor"
                  << std::endl;
        return true;

      } else if (strcmp(typeid(*param).name(),
                        typeid(al::Trigger).name()) == 0) { // Trigger
        // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
        assert(1 == 0); // Implement!
      } else {
        assert(1 == 0); // Implement!
      }
    }
  } else if (command == ParameterConfigureType::MAX) {
    ParameterValue v;
    if (conf.configurationvalue().Is<ParameterValue>()) {
      conf.configurationvalue().UnpackTo(&v);
      if (strcmp(typeid(*param).name(), typeid(al::ParameterBool).name()) ==
          0) {
        al::ParameterBool *p = dynamic_cast<al::ParameterBool *>(param);
        p->max(v.valuefloat());
      } else if (strcmp(typeid(*param).name(), typeid(al::Parameter).name()) ==
                 0) {
        al::Parameter *p = dynamic_cast<al::Parameter *>(param);
        p->max(v.valuefloat());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterInt).name()) == 0) {
        al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param);
        std::cout << " max change from " << src->valueSource()->port
                  << std::endl;
        p->max(v.valueint32());
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterString).name()) == 0) { //
        //                al::ParameterString *p =
        //                dynamic_cast<al::ParameterString *>(param);
        //                p->set(v.valuestring());
        std::cout << "Unexpected maximum value configure for ParameterString"
                  << std::endl;
        return true;
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterPose).name()) ==
                 0) { // al::ParameterPose
        // al::ParameterPose *p = dynamic_cast<al::ParameterPose *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterMenu).name()) ==
                 0) { // al::ParameterMenu
        // al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterChoice).name()) ==
                 0) { // al::ParameterChoice
        std::cout << "Unexpected maximum value configure for ParameterChoice"
                  << std::endl;
        return true;
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterVec3).name()) ==
                 0) { // al::ParameterVec3
        // al::ParameterVec3 *p = dynamic_cast<al::ParameterVec3 *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterVec4).name()) ==
                 0) { // al::ParameterVec4
        // al::ParameterVec4 *p = dynamic_cast<al::ParameterVec4 *>(param);
        assert(1 == 0); // Implement!
      } else if (strcmp(typeid(*param).name(),
                        typeid(al::ParameterColor).name()) ==
                 0) { // al::ParameterColor
        std::cout << "Unexpected maximum value configure for ParameterColor"
                  << std::endl;
        return true;

      } else if (strcmp(typeid(*param).name(),
                        typeid(al::Trigger).name()) == 0) { // Trigger
        // al::Trigger *p = dynamic_cast<al::Trigger *>(param);
        assert(1 == 0); // Implement!
      } else {
        assert(1 == 0); // Implement!
      }
    }
  }
  return true;
}

bool TincProtocol::processConfigureParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  bool ret = true;
  if (details->Is<ConfigureParameter>()) {
    ConfigureParameter conf;
    details->UnpackTo(&conf);
    auto addr = conf.id();
    for (auto *dim : mParameterSpaceDimensions) {
      if (addr == dim->getFullAddress()) {
        ret &= processParameterConfigure(conf, &dim->parameter(), src);
      }
    }

    for (auto *ps : mParameterSpaces) {
      for (auto dim : ps->getDimensions()) {
        if (addr == dim->getFullAddress()) {
          ret &= processParameterConfigure(conf, &dim->parameter(), src);
        }
      }
    }

    for (auto *param : mParameters) {
      if (addr == param->getFullAddress()) {
        ret &= processParameterConfigure(conf, param, src);
      }
    }
  } else {
    std::cerr << "Unexpected payload in Configure Parameter command"
              << std::endl;
  }
  return ret;
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
    return processCommandParameter(any, src);
  case PROCESSOR:
  //    return sendProcessors(src);
  case DISK_BUFFER:
    //          return processConfigureDiskBuffer(any, src);
    break;
  case DATA_POOL:
    return processCommandDataPool(any, src);
  case PARAMETER_SPACE:
    return processCommandParameterSpace(any, src);
    break;
  }
  return false;
}

bool TincProtocol::processCommandParameter(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << "Error: Invalid payload for Command" << std::endl;
    return false;
  }
  Command command;
  details->UnpackTo(&command);
  uint32_t commandNumber = command.message_id();
  if (command.details().Is<ParameterRequestChoiceElements>()) {

    std::vector<std::string> elements;
    auto id = command.id().id();
    for (auto *param : mParameters) {
      if (param->getFullAddress() == id) {
        if (strcmp(typeid(*param).name(), typeid(al::ParameterChoice).name()) ==
            0) {
          elements = dynamic_cast<al::ParameterChoice *>(param)->getElements();
          break;
        }
      }
    }
    for (auto *ps : mParameterSpaces) {
      for (auto dim : ps->getDimensions()) {
        if (dim->getFullAddress() == id) {
          if (strcmp(typeid(dim->parameter()).name(),
                     typeid(al::ParameterChoice).name()) == 0) {
            elements = dynamic_cast<al::ParameterChoice *>(&dim->parameter())
                           ->getElements();
            break;
          }
        }
      }
    }
    for (auto dim : mParameterSpaceDimensions) {
      if (dim->getFullAddress() == id) {
        if (strcmp(typeid(dim->parameter()).name(),
                   typeid(al::ParameterChoice).name()) == 0) {
          elements = dynamic_cast<al::ParameterChoice *>(&dim->parameter())
                         ->getElements();
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

    sendProtobufMessage(&msg, src);
    return true;
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

bool TincProtocol::processCommandParameterSpace(void *any, al::Socket *src) {
  google::protobuf::Any *details = static_cast<google::protobuf::Any *>(any);
  if (!details->Is<Command>()) {
    std::cerr << "Error: Invalid payload for Command" << std::endl;
    return false;
  }
  Command command;
  details->UnpackTo(&command);
  uint32_t commandNumber = command.message_id();
  if (command.details().Is<ParameterSpaceRequestCurrentPath>()) {
    ParameterSpaceRequestCurrentPath request;
    command.details().UnpackTo(&request);
    auto psId = command.id().id();
    for (auto ps : mParameterSpaces) {
      if (ps->getId() == psId) {
        auto curDir = ps->currentRunPath();
        // Send slice name
        std::cout << commandNumber << ":****: " << curDir << std::endl;
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

        sendProtobufMessage(&msg, src);
        return true;
      }
    }
  } else if (command.details().Is<ParameterSpaceRequestRootPath>()) {
    ParameterSpaceRequestRootPath request;
    command.details().UnpackTo(&request);
    auto psId = command.id().id();
    for (auto ps : mParameterSpaces) {
      if (ps->getId() == psId) {
        auto rootPath = ps->rootPath;
        // Send slice name
        std::cout << commandNumber << ":****: " << rootPath << std::endl;
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

        sendProtobufMessage(&msg, src);
        return true;
      }
    }
  }
  return false;
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

    sendTincMessage(&msg, nullptr, src);
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
    sendTincMessage(&msg, nullptr, src);
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
    sendTincMessage(&msg, nullptr, src);
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
    sendTincMessage(&msg, nullptr, src);
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
    sendTincMessage(&msg, nullptr, src);
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
    std::cerr << "Failed to send: " << buffer << "(" << strerror(errno) << ")"
              << std::endl;
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
