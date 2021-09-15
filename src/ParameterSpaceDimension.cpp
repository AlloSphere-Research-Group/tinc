#include "tinc/ParameterSpaceDimension.hpp"

#include "al/ui/al_DiscreteParameterValues.hpp"

#include <algorithm>
#include <limits>
#include <numeric> // std::iota

using namespace tinc;

al::VariantType dataTypeForParam(al::ParameterMeta *param) {
  if (dynamic_cast<al::Parameter *>(param)) {
    return al::VariantType::VARIANT_FLOAT;
  } else if (dynamic_cast<al::ParameterBool *>(param)) {
    return al::VariantType::VARIANT_BOOL;
  } else if (dynamic_cast<al::ParameterString *>(param)) {
    return al::VariantType::VARIANT_STRING;
  } else if (dynamic_cast<al::ParameterInt *>(param)) {
    return al::VariantType::VARIANT_INT32;
  } else if (dynamic_cast<al::ParameterInt8 *>(param)) {
    return al::VariantType::VARIANT_INT8;
  } else if (dynamic_cast<al::ParameterInt16 *>(param)) {
    return al::VariantType::VARIANT_INT16;
  } else if (dynamic_cast<al::ParameterInt64 *>(param)) {
    return al::VariantType::VARIANT_INT64;
  } else if (dynamic_cast<al::ParameterUInt8 *>(param)) {
    return al::VariantType::VARIANT_UINT8;
  } else if (dynamic_cast<al::ParameterUInt16 *>(param)) {
    return al::VariantType::VARIANT_UINT16;
  } else if (dynamic_cast<al::ParameterUInt32 *>(param)) {
    return al::VariantType::VARIANT_UINT32;
  } else if (dynamic_cast<al::ParameterUInt64 *>(param)) {
    return al::VariantType::VARIANT_UINT64;
  } else if (dynamic_cast<al::ParameterDouble *>(param)) {
    return al::VariantType::VARIANT_DOUBLE;
  } else if (dynamic_cast<al::ParameterVec3 *>(param)) {
    return al::VariantType::VARIANT_FLOAT;
  } else if (dynamic_cast<al::ParameterVec4 *>(param)) {
    return al::VariantType::VARIANT_FLOAT;
  } else if (dynamic_cast<al::ParameterColor *>(param)) {
    return al::VariantType::VARIANT_FLOAT;
  } else if (dynamic_cast<al::ParameterPose *>(param)) {
    return al::VariantType::VARIANT_DOUBLE;
  } else if (dynamic_cast<al::ParameterMenu *>(param)) {
    return al::VariantType::VARIANT_INT32;
  } else if (dynamic_cast<al::ParameterChoice *>(param)) {
    return al::VariantType::VARIANT_UINT64;
  } else if (dynamic_cast<al::ParameterMenu *>(param)) {
    return al::VariantType::VARIANT_INT32;
  } else if (dynamic_cast<al::Trigger *>(param)) {
    return al::VariantType::VARIANT_BOOL;
  }
  // FIXME complete implementation
  assert(0 == 1);
  return al::VariantType::VARIANT_FLOAT;
}

ParameterSpaceDimension::ParameterSpaceDimension(std::string name,
                                                 std::string group,
                                                 ParameterType paramType) {
  mParamInternal = true;
  mParamType = paramType;

  // FIXME define how we will handle all data types
  // FIXME ML support all types. String, bool left
  switch (paramType) {
  case ParameterType::PARAMETER_FLOAT:
    mParameterValue = new al::Parameter(name, group);
    break;
  case ParameterType::PARAMETER_DOUBLE:
    mParameterValue = new al::ParameterDouble(name, group);
    break;
  case ParameterType::PARAMETER_INT8:
    mParameterValue = new al::ParameterInt8(name, group);
    break;
  case ParameterType::PARAMETER_INT16:
    mParameterValue = new al::ParameterInt16(name, group);
    break;
  case ParameterType::PARAMETER_INT32:
    mParameterValue = new al::ParameterInt(name, group);
    break;
  case ParameterType::PARAMETER_INT64:
    mParameterValue = new al::ParameterInt64(name, group);
    break;
  case ParameterType::PARAMETER_UINT8:
    mParameterValue = new al::ParameterUInt8(name, group);
    break;
  case ParameterType::PARAMETER_UINT16:
    mParameterValue = new al::ParameterUInt16(name, group);
    break;
  case ParameterType::PARAMETER_UINT32:
    mParameterValue = new al::ParameterUInt32(name, group);
    break;
  case ParameterType::PARAMETER_UINT64:
    mParameterValue = new al::ParameterUInt64(name, group);
    break;
  case ParameterType::PARAMETER_MENU:
    mParameterValue = new al::ParameterMenu(name, group);
    break;
  default:
    throw std::invalid_argument("Data type for dimension not supported.");
  }
  mSpaceValues = std::make_unique<al::DiscreteParameterValues>(
      dataTypeForParam(mParameterValue));
}

ParameterSpaceDimension::ParameterSpaceDimension(al::ParameterMeta *param,
                                                 bool makeInternal) {
  mParamInternal = makeInternal;
  if (makeInternal) {
    if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
      mParameterValue = new al::Parameter(*p);
      mParamType = ParameterType::PARAMETER_FLOAT;
    } else if (al::ParameterBool *p =
                   dynamic_cast<al::ParameterBool *>(param)) {
      mParameterValue = new al::ParameterBool(*p);
      mParamType = ParameterType::PARAMETER_BOOL;
    } else if (al::ParameterString *p =
                   dynamic_cast<al::ParameterString *>(param)) {
      mParameterValue = new al::ParameterString(*p);
      mParamType = ParameterType::PARAMETER_STRING;
    } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
      mParameterValue = new al::ParameterInt(*p);
      mParamType = ParameterType::PARAMETER_INT32;
    } else if (al::ParameterInt8 *p =
                   dynamic_cast<al::ParameterInt8 *>(param)) {
      mParameterValue = new al::ParameterInt8(*p);
      mParamType = ParameterType::PARAMETER_INT8;
    } else if (al::ParameterInt16 *p =
                   dynamic_cast<al::ParameterInt16 *>(param)) {
      mParameterValue = new al::ParameterInt16(*p);
      mParamType = ParameterType::PARAMETER_INT16;
    } else if (al::ParameterInt64 *p =
                   dynamic_cast<al::ParameterInt64 *>(param)) {
      mParameterValue = new al::ParameterInt64(*p);
      mParamType = ParameterType::PARAMETER_INT64;
    } else if (al::ParameterUInt8 *p =
                   dynamic_cast<al::ParameterUInt8 *>(param)) {
      mParameterValue = new al::ParameterUInt8(*p);
      mParamType = ParameterType::PARAMETER_UINT8;
    } else if (al::ParameterUInt16 *p =
                   dynamic_cast<al::ParameterUInt16 *>(param)) {
      mParameterValue = new al::ParameterUInt16(*p);
      mParamType = ParameterType::PARAMETER_UINT16;
    } else if (al::ParameterUInt32 *p =
                   dynamic_cast<al::ParameterUInt32 *>(param)) {
      mParameterValue = new al::ParameterUInt32(*p);
      mParamType = ParameterType::PARAMETER_UINT32;
    } else if (al::ParameterUInt64 *p =
                   dynamic_cast<al::ParameterUInt64 *>(param)) {
      mParameterValue = new al::ParameterUInt64(*p);
      mParamType = ParameterType::PARAMETER_UINT64;
    } else if (al::ParameterDouble *p =
                   dynamic_cast<al::ParameterDouble *>(param)) {
      mParameterValue = new al::ParameterDouble(*p);
      mParamType = ParameterType::PARAMETER_DOUBLE;
    } else if (al::ParameterVec3 *p =
                   dynamic_cast<al::ParameterVec3 *>(param)) {
      mParameterValue = new al::ParameterVec3(*p);
      mParamType = ParameterType::PARAMETER_VEC3F;
    } else if (al::ParameterVec4 *p =
                   dynamic_cast<al::ParameterVec4 *>(param)) {
      mParameterValue = new al::ParameterVec4(*p);
      mParamType = ParameterType::PARAMETER_VEC4F;
    } else if (al::ParameterColor *p =
                   dynamic_cast<al::ParameterColor *>(param)) {
      mParameterValue = new al::ParameterColor(*p);
      mParamType = ParameterType::PARAMETER_COLORF;
    } else if (al::ParameterPose *p =
                   dynamic_cast<al::ParameterPose *>(param)) {
      mParameterValue = new al::ParameterPose(*p);
      mParamType = ParameterType::PARAMETER_POSED;
    } else if (al::ParameterMenu *p =
                   dynamic_cast<al::ParameterMenu *>(param)) {
      mParameterValue =
          new al::ParameterMenu(p->getName(), p->getGroup(), p->getDefault());
      dynamic_cast<al::ParameterMenu *>(mParameterValue)
          ->setElements(p->getElements());
      dynamic_cast<al::ParameterMenu *>(mParameterValue)->set(p->get());
      mParamType = ParameterType::PARAMETER_MENU;
    } else if (al::ParameterChoice *p =
                   dynamic_cast<al::ParameterChoice *>(param)) {
      mParameterValue = new al::ParameterChoice(*p);
      mParamType = ParameterType::PARAMETER_CHOICE;
    } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
      mParameterValue = new al::Trigger(*p);
      mParamType = ParameterType::PARAMETER_TRIGGER;
    } else {
      std::cerr << __FUNCTION__ << ": Unsupported Parameter Type" << std::endl;
    }
  } else {
    mParameterValue = param;
  }
  mSpaceValues = std::make_unique<al::DiscreteParameterValues>(
      dataTypeForParam(mParameterValue));
}

size_t ParameterSpaceDimension::size() { return mSpaceValues->size(); }

void ParameterSpaceDimension::sort(al::Socket *src) {

  std::vector<size_t> sortedIndeces(mSpaceValues->size());
  std::iota(sortedIndeces.begin(), sortedIndeces.end(), 0);

  if (mParamType == ParameterType::PARAMETER_FLOAT) {
    auto values = mSpaceValues->getValues<float>();
    std::stable_sort(
        sortedIndeces.begin(), sortedIndeces.end(),
        [&values](size_t i1, size_t i2) { return values[i1] < values[i2]; });

    std::vector<float> sortedValues(mSpaceValues->size());
    auto ids = mSpaceValues->getIds();
    std::vector<std::string> sortedIds(ids.size());
    assert(sortedIds.size() == 0 || sortedIds.size() == sortedValues.size());
    for (size_t i = 0; i < mSpaceValues->size(); i++) {
      sortedValues[i] = values[sortedIndeces[i]];
      if (sortedIds.size() > 0) {
        sortedIds[i] = ids[sortedIndeces[i]];
      }
    }
    setSpaceValues(sortedValues, "", src);
    setSpaceIds(sortedIds, src);
  } else if (mParamType == ParameterType::PARAMETER_INT8) {

    auto values = mSpaceValues->getValues<int8_t>();
    std::stable_sort(
        sortedIndeces.begin(), sortedIndeces.end(),
        [&values](size_t i1, size_t i2) { return values[i1] < values[i2]; });

    std::vector<int8_t> sortedValues(mSpaceValues->size());
    auto ids = mSpaceValues->getIds();
    std::vector<std::string> sortedIds(ids.size());
    assert(sortedIds.size() == 0 || sortedIds.size() == sortedValues.size());
    for (size_t i = 0; i < mSpaceValues->size(); i++) {
      sortedValues[i] = values[sortedIndeces[i]];
      if (sortedIds.size() > 0) {
        sortedIds[i] = ids[sortedIndeces[i]];
      }
    }
    setSpaceValues(sortedValues, "", src);
    setSpaceIds(sortedIds, src);
  } else {
  }
}

void ParameterSpaceDimension::clear(al::Socket *src) {
  mSpaceValues->clear();
  //  conformSpace();
  onDimensionMetadataChange(this, src);
}

float ParameterSpaceDimension::at(size_t index) {
  float value = 0.0;
  if (size() > 0) {
    value = mSpaceValues->at(index);
  }
  return value;
}

std::string ParameterSpaceDimension::idAt(size_t index) {
  return mSpaceValues->idAt(index);
}

std::vector<std::string> ParameterSpaceDimension::getIdsForValue(float value) {
  return mSpaceValues->getIdsForValue(value);
}

float ParameterSpaceDimension::getCurrentValue() {
  if (mSpaceValues->size() > 0) {
    return mSpaceValues->at(getCurrentIndex());
  } else {
    return mParameterValue->toFloat();
  }
}

bool ParameterSpaceDimension::setCurrentValue(float value) {
  return mParameterValue->fromFloat(value);
}

size_t ParameterSpaceDimension::getCurrentIndex() {
  // TODO avoid converting to float
  return getIndexForValue(mParameterValue->toFloat());
}

std::vector<size_t> ParameterSpaceDimension::getCurrentIndeces() {
  return mSpaceValues->getIndecesForValue(getCurrentValue());
}

void ParameterSpaceDimension::setCurrentIndex(size_t index) {
  // TODO avoid converting to float
  mParameterValue->fromFloat(mSpaceValues->at(index));
}

std::string ParameterSpaceDimension::getCurrentId() {
  return mSpaceValues->idAt(getCurrentIndex());
}

std::vector<std::string> ParameterSpaceDimension::getCurrentIds() {
  if (mSpaceValues->getIds().size() > 0) {
    return mSpaceValues->getIdsForValue(getCurrentValue());
  } else {
    return {};
  }
}

size_t ParameterSpaceDimension::getIndexForValue(float value) {
  return mSpaceValues->getIndexForValue(value);
}

ParameterSpaceDimension::~ParameterSpaceDimension() {
  if (mParamInternal && mParameterValue) {
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
  if (mSpaceValues->size() < 2) {
    std::cout << "WARNING: no space set " << __FUNCTION__ << " " << __FILE__
              << ":" << __LINE__ << std::endl;
    return;
  }
  size_t curIndex = getCurrentIndex();
  float temp = mParameterValue->toFloat();
  size_t stride = mSpaceValues->stride();
  if (curIndex < mSpaceValues->size() - stride) {
    // Check if we have an element above to compare to
    float nextTemp = mSpaceValues->at(curIndex + stride);
    if (nextTemp > temp) {
      // Element above is greater, so increment index and load
      setCurrentIndex(curIndex + stride);
    } else if (curIndex >= stride) {
      // If element above is not greater and we have  space
      setCurrentIndex(curIndex - stride);
    }
  } else {
    float nextTemp = mSpaceValues->at(curIndex - stride);
    while (nextTemp == temp) {
      curIndex--;
      if (curIndex == 0) {
        //            we are at the last index
        return;
      }
      nextTemp = mSpaceValues->at(curIndex);
    }
    if (nextTemp > temp) {
      setCurrentIndex(curIndex);
    }
  }
}

void ParameterSpaceDimension::stepDecrease() {
  if (mSpaceValues->size() < 2) {
    std::cout << "WARNING: no space set " << __FUNCTION__ << " " << __FILE__
              << ":" << __LINE__ << std::endl;
    return;
  }
  size_t curIndex = getCurrentIndex();
  float temp = mParameterValue->toFloat();
  if (curIndex > 0) {
    // Check if we have an element above to compare to
    float nextTemp = mSpaceValues->at(curIndex - 1);
    if (nextTemp < temp) {
      // Element below is smaller, so decrement index
      setCurrentIndex(curIndex - 1);
    } else if (curIndex < mSpaceValues->size() - 1) {
      // If element below is not smaller and we have  space
      setCurrentIndex(curIndex + 1);
    }
  } else {
    if (mSpaceValues->at(1) < temp) {
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
  if (count == 0 && mSpaceValues->size() == 0) {
    return;
  }
  mSpaceValues->clear();
  mSpaceValues->append(values, count, idprefix);
  onDimensionMetadataChange(this, src);
}

void ParameterSpaceDimension::appendSpaceValues(void *values, size_t count,
                                                std::string idprefix,
                                                al::Socket *src) {
  // TODO add safety check for types and pointer sizes
  mSpaceValues->append(values, count, idprefix);
  onDimensionMetadataChange(this, src);
}

void ParameterSpaceDimension::setSpaceIds(std::vector<std::string> ids,
                                          al::Socket *src) {
  mSpaceValues->setIds(ids);
  onDimensionMetadataChange(this, src);
}

std::vector<std::string> ParameterSpaceDimension::getSpaceIds() {
  return mSpaceValues->getIds();
}

namespace tinc {

template <class DataType, class ParamType>
void _setLimits(ParameterSpaceDimension *dim, ParamType *param,
                al::Socket *src) {
  DataType max = std::numeric_limits<DataType>::lowest();
  DataType min = std::numeric_limits<DataType>::max();
  for (auto value : dim->getSpaceValues<DataType>()) {
    if (value > max) {
      max = value;
    }
    if (value < min) {
      min = value;
    }
  }
  if (dim->getSpaceValues<DataType>().size() == 0) {
    max = std::numeric_limits<DataType>::max();
    min = std::numeric_limits<DataType>::lowest();
  }
  param->max(max, src->valueSource());
  param->min(min, src->valueSource());
  if (param->get() < min && min != std::numeric_limits<float>::max()) {
    param->set(min, src->valueSource());
  } else if (param->get() > max &&
             max != std::numeric_limits<float>::lowest()) {
    param->set(max, src->valueSource());
  }
}

} // namespace tinc

bool ParameterSpaceDimension::conformSpace(al::Socket *src) {
  sort(src);
  // TODO we should also validate stride in this function
  //
  // TODO use getFields to determine type of data:
  //  auto values = p->getFields(fields);

  switch (mSpaceValues->getDataType()) {
  case al::VariantType::VARIANT_FLOAT: {
    auto &param = getParameter<al::Parameter>();
    _setLimits<float>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_DOUBLE: {
    auto &param = getParameter<al::ParameterDouble>();
    _setLimits<double>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_INT8: {
    auto &param = getParameter<al::ParameterInt8>();
    _setLimits<int8_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_UINT8: {
    auto &param = getParameter<al::ParameterUInt8>();
    _setLimits<uint8_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_INT16: {
    auto &param = getParameter<al::ParameterInt16>();
    _setLimits<int16_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_UINT16: {
    auto &param = getParameter<al::ParameterUInt16>();
    _setLimits<uint16_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_INT32: {
    auto &param = getParameter<al::ParameterInt>();
    _setLimits<int32_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_UINT32: {
    auto &param = getParameter<al::ParameterUInt32>();
    _setLimits<uint32_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_INT64: {
    auto &param = getParameter<al::ParameterInt64>();
    _setLimits<int64_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_UINT64: {
    auto &param = getParameter<al::ParameterUInt64>();
    _setLimits<uint64_t>(this, &param, src);
  } break;
  case al::VariantType::VARIANT_BOOL: {
    auto &param = getParameter<al::ParameterBool>();
    _setLimits<float>(this, &param, src);
  } break;

    // FIXME ML complete support for all types. Done.

    //  case al::DiscreteParameterValues::DOUBLE: {
    //    valueDbl = *(static_cast<double *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::BOOL: {
    //    valueDbl = *(static_cast<uint8_t *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::INT64: {
    //    valueDbl = *(static_cast<int64_t *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::UINT64: {
    //    valueDbl = *(static_cast<uint64_t *>(value));
    //  } break;
    //  case al::DiscreteParameterValues::STRING: {
    //    valueDbl = *(static_cast<uint64_t *>(value));
    //  } break;
  }
  return true;
}

std::shared_ptr<ParameterSpaceDimension> ParameterSpaceDimension::deepCopy() {
  auto dimCopy = std::make_shared<ParameterSpaceDimension>(
      getName(), getGroup(), mParamType);
  mSpaceValues->lock();
  dimCopy->mSpaceValues->append(mSpaceValues->getValuesPtr(),
                                mSpaceValues->size());
  dimCopy->mSpaceValues->setIds(mSpaceValues->getIds());
  mSpaceValues->unlock();
  dimCopy->setCurrentIndex(getCurrentIndex());
  return dimCopy;
}

void ParameterSpaceDimension::print(std::ostream &stream) {
  stream << "ParameterSpaceParameterSpaceDimension: " << this->getFullAddress()
         << "  @" << this << std::endl;
  stream << "   ParameterMeta " << this->mParameterValue << std::endl;
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
