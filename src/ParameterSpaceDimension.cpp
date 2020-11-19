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
  case Datatype::INT32:
  case Datatype::UINT32:
    mParameterValue = new al::ParameterInt(name, group);
    break;
  }
}

ParameterSpaceDimension::Datatype dataTypeForParam(al::ParameterMeta *param) {
  if (dynamic_cast<al::Parameter *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  }
  if (dynamic_cast<al::ParameterBool *>(param)) {
    return ParameterSpaceDimension::Datatype::FLOAT;
  }
  if (dynamic_cast<al::ParameterInt *>(param)) {
    return ParameterSpaceDimension::Datatype::INT32;
  }
  if (dynamic_cast<al::ParameterMenu *>(param)) {
    return ParameterSpaceDimension::Datatype::INT32;
  }
  // FIXME complete implementation
  assert(0 == 1);
  return ParameterSpaceDimension::Datatype::FLOAT;
}

ParameterSpaceDimension::ParameterSpaceDimension(al::ParameterMeta *param)
    : mSpaceValues(dataTypeForParam(param)) {
  mParamInternal = false;
  mParameterValue = param;
}

size_t ParameterSpaceDimension::size() { return mSpaceValues.size(); }

void ParameterSpaceDimension::clear() {
  mSpaceValues.clear();
  onDimensionMetadataChange(this);
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

// size_t ParameterSpaceDimension::getFirstIndexForValue(float value,
//                                                      bool reverse) {
//  int paramIndex = -1;

//  if (!reverse) {
//    size_t i = 0;
//    for (auto it = mValues.begin(); it != mValues.end(); it++) {
//      if (*it == value) {
//        paramIndex = i;
//        break;
//      } else if (*it > value && (i == mValues.size() - 1)) {
//        break;
//      }
//      auto next = it;
//      std::advance(next, 1);
//      if (next != mValues.end()) {
//        if (*it > value && *next < value) { // space is sorted and descending
//          paramIndex = i;
//          break;
//        } else if (*it<value && * next>
//                       value) { // space is sorted and ascending
//          paramIndex = i + 1;
//          break;
//        }
//      }
//      i++;
//    }
//  } else {
//    if (mValues.size() > 0) {

//      value = mSpaceValues.at(getIndexForValue(value));
//      size_t i = mValues.size();
//      for (auto it = mValues.rbegin(); it != mValues.rend(); it++) {
//        if (*it == value) {
//          paramIndex = i;
//          break;
//        } else if (*it < value && (i == 0)) {
//          break;
//        }
//        auto next = it;
//        std::advance(next, 1);
//        if (next != mValues.rend()) {
//          if (*it<value && * next> value) { // space is sorted and descending
//            paramIndex = i;
//            break;
//          } else if (*it > value &&
//                     *next < value) { // space is sorted and ascending
//            paramIndex = i - 1;
//            break;
//          }
//        }
//        i--;
//      }
//    }
//  }
//  if (paramIndex < 0) {
//    //          std::cerr << "WARNING: index not found" << std::endl;
//    paramIndex = 0;
//  }
//  return paramIndex;
//}

// size_t ParameterSpaceDimension::getFirstIndexForId(std::string id,
//                                                   bool reverse) {
//  size_t paramIndex = std::numeric_limits<size_t>::max();

//  if (!reverse) {
//    size_t i = 0;
//    for (auto it = mIds.begin(); it != mIds.end(); it++) {
//      if (*it == id) {
//        paramIndex = i;
//        break;
//      }
//      i++;
//    }
//  } else {
//    size_t i = mIds.size() - 1;
//    for (auto it = mIds.rbegin(); it != mIds.rend(); it++) {
//      if (*it == id) {
//        paramIndex = i;
//        break;
//      }
//      i--;
//    }
//  }
//  return paramIndex;
//}

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

// std::vector<std::string> ParameterSpaceDimension::getAllCurrentIds() {
//  float value = getCurrentValue();
//  return getAllIds(value);
//}

// std::vector<std::string> ParameterSpaceDimension::getAllIds(float value) {
//  size_t lowIndex = getFirstIndexForValue(value);
//  size_t highIndex = getFirstIndexForValue(
//      value, true); // Open range value (excluded from range)
//  std::vector<std::string> ids;

//  for (size_t i = lowIndex; i < highIndex; i++) {
//    ids.push_back(idAt(i));
//  }
//  if (lowIndex ==
//      highIndex) { // Hack... shouldn't need to be done. This should be
//      fixed
//                   // for one dimensional data parameter spaces.
//    ids.push_back(idAt(lowIndex));
//  }
//  return ids;
//}

// std::vector<size_t> ParameterSpaceDimension::getAllCurrentIndeces() {
//  float value = getCurrentValue();
//  return getAllIndeces(value);
//}

// std::vector<size_t> ParameterSpaceDimension::getAllIndeces(float value) {
//  size_t lowIndex = getFirstIndexForValue(value);
//  size_t highIndex = getFirstIndexForValue(
//      value, true); // Open range value (excluded from range)
//  std::vector<size_t> idxs;

//  for (size_t i = lowIndex; i < highIndex; i++) {
//    idxs.push_back(i);
//  }
//  if (lowIndex ==
//      highIndex) { // Hack... shouldn't need to be done. This should be
//      fixed
//                   // for one dimensional data parameter spaces.
//    idxs.push_back(lowIndex);
//  }
//  return idxs;
//}

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
                                             bool propagate) {
  // TODO add safety check for types and pointer sizes
  mSpaceValues.clear();
  mSpaceValues.append(values, count, idprefix);
  if (propagate) {
    onDimensionMetadataChange(this);
  }
}

void ParameterSpaceDimension::setSpaceValues(std::vector<float> values,
                                             std::string idprefix,
                                             bool propagate) {
  mSpaceValues.clear();
  // TODO add safety check for types and pointer sizes
  mSpaceValues.append(values.data(), values.size(), idprefix);
  if (propagate) {
    onDimensionMetadataChange(this);
  }
}

void ParameterSpaceDimension::appendSpaceValues(void *values, size_t count,
                                                std::string idprefix,
                                                bool propagate) {
  // TODO add safety check for types and pointer sizes
  mSpaceValues.append(values, count, idprefix);
  if (propagate) {
    onDimensionMetadataChange(this);
  }
}

void ParameterSpaceDimension::setSpaceIds(std::vector<std::string> ids,
                                          bool propagate) {
  mSpaceValues.setIds(ids);
  if (propagate) {
    onDimensionMetadataChange(this);
  }
}

std::vector<std::string> ParameterSpaceDimension::getSpaceIds() {
  return mSpaceValues.getIds();
}

void ParameterSpaceDimension::conformSpace() {
  switch (mSpaceValues.getDataType()) {
  case al::DiscreteParameterValues::FLOAT: {
    auto &param = parameter<al::Parameter>();
    float max = std::numeric_limits<float>::min();
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
    param.setNoCalls(at(0));
  } break;
  case al::DiscreteParameterValues::INT8: {
    auto &param = parameter<al::ParameterInt>();
    float max = std::numeric_limits<int8_t>::min();
    float min = std::numeric_limits<int8_t>::max();
    for (auto value : getSpaceValues<int8_t>()) {
      if (value > max) {
        max = value;
      }
      if (value < min) {
        min = value;
      }
    }
    param.setNoCalls(at(0));
  } break;
  case al::DiscreteParameterValues::INT32: {
    auto &param = parameter<al::ParameterInt>();
    float max = std::numeric_limits<int32_t>::min();
    float min = std::numeric_limits<int32_t>::max();
    for (auto value : getSpaceValues<int32_t>()) {
      if (value > max) {
        max = value;
      }
      if (value < min) {
        min = value;
      }
    }
    param.setNoCalls(at(0));
  } break;
  case al::DiscreteParameterValues::UINT32: {
    auto &param = parameter<al::ParameterInt>();
    float max = std::numeric_limits<uint32_t>::min();
    float min = std::numeric_limits<uint32_t>::max();
    for (auto value : getSpaceValues<int32_t>()) {

      if (value > param.max()) {
        param.max(value);
      }
      if (value < param.min()) {
        param.min(value);
      }
    }
    param.setNoCalls(at(0));
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
