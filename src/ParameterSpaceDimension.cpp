#include "tinc/ParameterSpaceDimension.hpp"

#include "al/ui/al_DiscreteParameterValues.hpp"

#include <limits>

using namespace tinc;

ParameterSpaceDimension::ParameterSpaceDimension(
    std::string name, std::string group,
    ParameterSpaceDimension::Datatype dataType)
    : mSpaceValues(dataType) {
  // FIXME define how we will handle all data types
  mParamInternal = true; // FIXME crash if unsupported data type on destrcutor
  switch (dataType) {
  case Datatype::FLOAT:
    mParameterValue = new al::Parameter(name, group);
    break;
  case Datatype::UINT8:
  case Datatype::INT8:
  case Datatype::INT32:
    mParameterValue = new al::ParameterInt(name, group);
    break;
  }
}

ParameterSpaceDimension::Datatype dataTypeForParam(al::ParameterMeta *param) {
  if (dynamic_cast<al::Parameter *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  } else if (dynamic_cast<al::ParameterBool *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  } else if (dynamic_cast<al::ParameterString *>(param)) {
    return ParameterSpaceDimension::Datatype::STRING;
  } else if (dynamic_cast<al::ParameterInt *>(param)) {
    return ParameterSpaceDimension::Datatype::INT32;
  } else if (dynamic_cast<al::ParameterVec3 *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  } else if (dynamic_cast<al::ParameterVec4 *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  } else if (dynamic_cast<al::ParameterColor *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  } else if (dynamic_cast<al::ParameterPose *>(param)) {
    return ParameterSpaceDimension::Datatype::DOUBLE;
  } else if (dynamic_cast<al::ParameterMenu *>(param)) {
    return ParameterSpaceDimension::Datatype::INT32;
  } else if (dynamic_cast<al::ParameterChoice *>(param)) {
    return ParameterSpaceDimension::Datatype::UINT64;
  } else if (dynamic_cast<al::Trigger *>(param)) {
    return ParameterSpaceDimension::Datatype::BOOL;
  }
  // FIXME complete implementation
  assert(0 == 1);
  return ParameterSpaceDimension::Datatype::FLOAT;
}

ParameterSpaceDimension::ParameterSpaceDimension(al::ParameterMeta *param,
                                                 bool makeInternal)
    : mSpaceValues(dataTypeForParam(param)) {
  mParamInternal = makeInternal;
  if (makeInternal) {
    if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
      mParameterValue = new al::Parameter(*p);
    } else if (al::ParameterBool *p =
                   dynamic_cast<al::ParameterBool *>(param)) {
      mParameterValue = new al::ParameterBool(*p);
    } else if (al::ParameterString *p =
                   dynamic_cast<al::ParameterString *>(param)) {
      mParameterValue = new al::ParameterString(*p);
    } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
      mParameterValue = new al::ParameterInt(*p);
    } else if (al::ParameterVec3 *p =
                   dynamic_cast<al::ParameterVec3 *>(param)) {
      mParameterValue = new al::ParameterVec3(*p);
    } else if (al::ParameterVec4 *p =
                   dynamic_cast<al::ParameterVec4 *>(param)) {
      mParameterValue = new al::ParameterVec4(*p);
    } else if (al::ParameterColor *p =
                   dynamic_cast<al::ParameterColor *>(param)) {
      mParameterValue = new al::ParameterColor(*p);
    } else if (al::ParameterPose *p =
                   dynamic_cast<al::ParameterPose *>(param)) {
      mParameterValue = new al::ParameterPose(*p);
    } else if (al::ParameterMenu *p =
                   dynamic_cast<al::ParameterMenu *>(param)) {
      mParameterValue =
          new al::ParameterMenu(p->getName(), p->getGroup(), p->getDefault());
      dynamic_cast<al::ParameterMenu *>(mParameterValue)->set(p->get());
    } else if (al::ParameterChoice *p =
                   dynamic_cast<al::ParameterChoice *>(param)) {
      mParameterValue = new al::ParameterChoice(*p);
    } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
      mParameterValue = new al::Trigger(*p);
    } else {
      std::cerr << __FUNCTION__ << ": Unsupported Parameter Type" << std::endl;
    }
  } else {
    mParameterValue = param;
  }
}

size_t ParameterSpaceDimension::size() { return mSpaceValues.size(); }

void ParameterSpaceDimension::sort() {

  // FIXME implement sort
}

void ParameterSpaceDimension::clear(al::Socket *src) {
  mSpaceValues.clear();
  conformSpace();
  onDimensionMetadataChange(this, src);
}

float ParameterSpaceDimension::at(size_t index) {
  float value = 0.0;
  if (size() > 0) {
    value = mSpaceValues.at(index);
  }
  return value;
}

std::string ParameterSpaceDimension::idAt(size_t index) {
  return mSpaceValues.idAt(index);
}

float ParameterSpaceDimension::getCurrentValue() {
  if (mSpaceValues.size() > 0) {
    return mSpaceValues.at(getCurrentIndex());
  } else {
    return mParameterValue->toFloat();
  }
}

void ParameterSpaceDimension::setCurrentValue(float value) {
  mParameterValue->fromFloat(value);
}

size_t ParameterSpaceDimension::getCurrentIndex() {
  // TODO avoid converting to float
  return getIndexForValue(mParameterValue->toFloat());
}

void ParameterSpaceDimension::setCurrentIndex(size_t index) {
  // TODO avoid converting to float
  mParameterValue->fromFloat(mSpaceValues.at(index));
}

std::string ParameterSpaceDimension::getCurrentId() {
  return mSpaceValues.idAt(getCurrentIndex());
}

size_t ParameterSpaceDimension::getIndexForValue(float value) {
  return mSpaceValues.getIndexForValue(value);
}

ParameterSpaceDimension::~ParameterSpaceDimension() {
  if (mParamInternal) {
    delete mParameterValue;
  }
}

std::string ParameterSpaceDimension::getName() {
  return mParameterValue->getName();
}

std::string ParameterSpaceDimension::getGroup() {
  return mParameterValue->getGroup();
}

std::string ParameterSpaceDimension::getFullAddress() {
  return mParameterValue->getFullAddress();
}

void ParameterSpaceDimension::stepIncrement() {
  if (mSpaceValues.size() < 2) {
    std::cout << "WARNING: no space set " << __FUNCTION__ << " " << __FILE__
              << ":" << __LINE__ << std::endl;
    return;
  }
  size_t curIndex = getCurrentIndex();
  float temp = mParameterValue->toFloat();
  if (curIndex < mSpaceValues.size() - 1) {
    // Check if we have an element above to compare to
    float nextTemp = mSpaceValues.at(curIndex + 1);
    if (nextTemp > temp) {
      // Element above is greater, so increment index and load
      setCurrentIndex(curIndex + 1);
    } else if (curIndex > 0) {
      // If element above is not greater and we have  space
      setCurrentIndex(curIndex - 1);
    }
  } else {
    if (mSpaceValues.at(curIndex - 1) > temp) {
      setCurrentIndex(curIndex - 1);
    }
  }
}

void ParameterSpaceDimension::stepDecrease() {
  if (mSpaceValues.size() < 2) {
    std::cout << "WARNING: no space set " << __FUNCTION__ << " " << __FILE__
              << ":" << __LINE__ << std::endl;
    return;
  }
  size_t curIndex = getCurrentIndex();
  float temp = mParameterValue->toFloat();
  if (curIndex > 0) {
    // Check if we have an element above to compare to
    float nextTemp = mSpaceValues.at(curIndex - 1);
    if (nextTemp < temp) {
      // Element below is smaller, so decrement index
      setCurrentIndex(curIndex - 1);
    } else if (curIndex < mSpaceValues.size() - 1) {
      // If element below is not smaller and we have  space
      setCurrentIndex(curIndex + 1);
    }
  } else {
    if (mSpaceValues.at(1) < temp) {
      setCurrentIndex(1);
    }
  }
}

// void ParameterSpaceDimension::push_back(float value, std::string id) {

//  // FIXME There is no check to see if value is already present. Could cause
//  // trouble
//  // if value is there already.
//  mValues.emplace_back(value);

//  if (id.size() > 0) {
//    mIds.push_back(id);
//    if (mIds.size() != mValues.size()) {
//      std::cerr
//          << " ERROR! value and id mismatch in parameter space! This is
//          bad."
//          << std::endl;
//    }
//  }

//  if (value > mParameterValue.max()) {
//    mParameterValue.max(value);
//  }
//  if (value < mParameterValue.min()) {
//    mParameterValue.min(value);
//  }

//  onDimensionMetadataChange(this);
//}

void ParameterSpaceDimension::setSpaceValues(void *values, size_t count,
                                             std::string idprefix,
                                             al::Socket *src) {
  // TODO add safety check for types and pointer sizes
  if (count == 0 && mSpaceValues.size() == 0) {
    return;
  }
  mSpaceValues.clear();
  mSpaceValues.append(values, count, idprefix);
  conformSpace();
  onDimensionMetadataChange(this, src);
}

void ParameterSpaceDimension::appendSpaceValues(void *values, size_t count,
                                                std::string idprefix,
                                                al::Socket *src) {
  // TODO add safety check for types and pointer sizes
  mSpaceValues.append(values, count, idprefix);
  onDimensionMetadataChange(this, src);
}

void ParameterSpaceDimension::setSpaceIds(std::vector<std::string> ids,
                                          al::Socket *src) {
  mSpaceValues.setIds(ids);
  onDimensionMetadataChange(this, src);
}

std::vector<std::string> ParameterSpaceDimension::getSpaceIds() {
  return mSpaceValues.getIds();
}

void ParameterSpaceDimension::conformSpace() {
  switch (mSpaceValues.getDataType()) {
  case al::DiscreteParameterValues::FLOAT: {
    auto &param = getParameter<al::Parameter>();
    float max = std::numeric_limits<float>::lowest();
    float min = std::numeric_limits<float>::max();
    for (auto value : getSpaceValues<float>()) {
      if (value > max) {
        max = value;
      }
      if (value < min) {
        min = value;
      }
    }
    param.max(max);
    param.min(min);
    if (param.get() < min && min != std::numeric_limits<float>::max()) {
      param.set(min);
    } else if (param.get() > max &&
               max != std::numeric_limits<float>::lowest()) {
      param.set(max);
    }
  } break;
  case al::DiscreteParameterValues::UINT8: {
    auto &param = getParameter<al::ParameterInt>();
    uint8_t max = std::numeric_limits<uint8_t>::lowest();
    uint8_t min = std::numeric_limits<uint8_t>::max();
    for (auto value : getSpaceValues<uint8_t>()) {
      if (value > max) {
        max = value;
      }
      if (value < min) {
        min = value;
      }
    }
    param.max(max);
    param.min(min);
    if (param.get() < min && min != std::numeric_limits<uint8_t>::max()) {
      param.set(min);
    } else if (param.get() > max &&
               max != std::numeric_limits<uint8_t>::lowest()) {
      param.set(max);
    }
  } break;
  case al::DiscreteParameterValues::INT8: {
    auto &param = getParameter<al::ParameterInt>();
    int8_t max = std::numeric_limits<int8_t>::lowest();
    int8_t min = std::numeric_limits<int8_t>::max();
    for (auto value : getSpaceValues<int8_t>()) {
      if (value > max) {
        max = value;
      }
      if (value < min) {
        min = value;
      }
    }
    param.max(max);
    param.min(min);
    if (param.get() < min && min != std::numeric_limits<int8_t>::max()) {
      param.set(min);
    } else if (param.get() > max &&
               max != std::numeric_limits<int8_t>::lowest()) {
      param.set(max);
    }
  } break;
  case al::DiscreteParameterValues::INT32: {
    auto &param = getParameter<al::ParameterInt>();
    int32_t max = std::numeric_limits<int32_t>::lowest();
    int32_t min = std::numeric_limits<int32_t>::max();
    for (auto value : getSpaceValues<int32_t>()) {
      if (value > max) {
        max = value;
      }
      if (value < min) {
        min = value;
      }
    }
    param.max(max);
    param.min(min);
    if (param.get() < min && min != std::numeric_limits<int32_t>::max()) {
      param.set(min);
    } else if (param.get() > max &&
               max != std::numeric_limits<int32_t>::lowest()) {
      param.set(max);
    }
  } break;
  case al::DiscreteParameterValues::UINT32: {
    auto &param = getParameter<al::ParameterInt>();
    uint32_t max = std::numeric_limits<uint32_t>::lowest();
    uint32_t min = std::numeric_limits<uint32_t>::max();
    for (auto value : getSpaceValues<int32_t>()) {

      if (value > param.max()) {
        param.max(value);
      }
      if (value < param.min()) {
        param.min(value);
      }
    }
    param.max(max);
    param.min(min);
    if (param.get() < min && min != std::numeric_limits<int32_t>::max()) {
      param.set(min);
    } else if (param.get() > max &&
               max != std::numeric_limits<int32_t>::lowest()) {
      param.set(max);
    }
  } break;
    // FIXME complete support for all types

    //  case al::DiscreteParameterValues::DOUBLE: {
    //    valueDbl = *(static_cast<double *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::UINT8: {
    //    valueDbl = *(static_cast<uint8_t *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::INT64: {
    //    valueDbl = *(static_cast<int64_t *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::UINT64: {
    //    valueDbl = *(static_cast<uint64_t *>(value));
    //  } break;
  }
}

std::shared_ptr<ParameterSpaceDimension> ParameterSpaceDimension::deepCopy() {
  auto dimCopy = std::make_shared<ParameterSpaceDimension>(
      getName(), getGroup(), mSpaceValues.getDataType());
  mSpaceValues.lock();
  dimCopy->mSpaceValues.append(mSpaceValues.getValuesPtr(),
                               mSpaceValues.size());
  dimCopy->mSpaceValues.setIds(mSpaceValues.getIds());
  mSpaceValues.unlock();
  dimCopy->setCurrentIndex(getCurrentIndex());
  return dimCopy;
}

class sort_indices {
private:
  float *mFloatArray;

public:
  sort_indices(float *floatArray) : mFloatArray(floatArray) {}
  bool operator()(size_t i, size_t j) const {
    return mFloatArray[i] < mFloatArray[j];
  }
};

// FIXME ensure sort is available
// void ParameterSpaceDimension::sort() {
//  lock();

//  std::vector<size_t> indeces;
//  indeces.resize(mValues.size());
//  for (size_t i = 0; i < indeces.size(); i++) {
//    indeces[i] = i;
//  }
//  std::sort(indeces.begin(), indeces.end(), sort_indices(mValues.data()));
//  std::vector<float> newValues;
//  std::vector<std::string> newIds;
//  newValues.reserve(indeces.size());
//  newIds.reserve(indeces.size());
//  if (mValues.size() != mIds.size() && mIds.size() > 0) {
//    std::cerr << "ERROR: sort() will crash (or lead ot unexpected behavior) as
//    "
//                 "the size of values and ids don't match."
//              << std::endl;
//  }
//  for (size_t i = 0; i < indeces.size(); i++) {
//    size_t index = indeces[i];
//    newValues.push_back(mValues[index]);
//    if (mIds.size() > 0) {
//      newIds.push_back(mIds[index]);
//    }
//  }
//  mValues = newValues;
//  mIds = newIds;
//  unlock();
//  onDimensionMetadataChange(this);
//}
