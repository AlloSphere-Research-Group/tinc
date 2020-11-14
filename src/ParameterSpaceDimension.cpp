#include "tinc/ParameterSpaceDimension.hpp"

#include <limits>

using namespace tinc;

float ParameterSpaceDimension::at(size_t x) {
  if (x < mValues.size()) {
    return mValues[x];
  }
  return 0.0f;
}

std::string ParameterSpaceDimension::idAt(size_t x) {
  if (x < mIds.size()) {
    return mIds[x];
  } else if (mIds.size() == 0 && x < mValues.size()) {
    return std::to_string(mValues[x]);
  } else {
    return std::string();
  }
}

size_t ParameterSpaceDimension::size() { return mValues.size(); }

void ParameterSpaceDimension::clear() {
  lock();
  mParameterValue.min(FLT_MAX);
  mParameterValue.max(FLT_MIN);
  mValues.clear();
  mIds.clear();
  unlock();
  onDimensionMetadataChange(this);
}

size_t ParameterSpaceDimension::getIndexForValue(float value) {
  auto indeces = getAllIndeces(value);
  for (auto connectedSpace : mConnectedSpaces) {
    if (connectedSpace->size() > 0) {
      std::vector<std::string> ids =
          connectedSpace->getAllIds(connectedSpace->parameter().get());
      std::vector<size_t> commonIndeces;
      for (auto index : indeces) {
        auto id = idAt(index);
        if (std::find(ids.begin(), ids.end(), idAt(index)) != ids.end()) {
          commonIndeces.push_back(index);
        } else if (ids.size() == 1 && ids[0] == "./") {
          commonIndeces.push_back(index);
        }
      }
      indeces = commonIndeces;
    }
  }
  return indeces[0];
}

size_t ParameterSpaceDimension::getFirstIndexForValue(float value,
                                                      bool reverse) {
  int paramIndex = -1;

  if (!reverse) {
    size_t i = 0;
    for (auto it = mValues.begin(); it != mValues.end(); it++) {
      if (*it == value) {
        paramIndex = i;
        break;
      } else if (*it > value && (i == mValues.size() - 1)) {
        break;
      }
      auto next = it;
      std::advance(next, 1);
      if (next != mValues.end()) {
        if (*it > value && *next < value) { // space is sorted and descending
          paramIndex = i;
          break;
        } else if (*it<value && * next>
                       value) { // space is sorted and ascending
          paramIndex = i + 1;
          break;
        }
      }
      i++;
    }
  } else {
    if (mValues.size() > 0) {

      value = at(getFirstIndexForValue(value));
      size_t i = mValues.size();
      for (auto it = mValues.rbegin(); it != mValues.rend(); it++) {
        if (*it == value) {
          paramIndex = i;
          break;
        } else if (*it < value && (i == 0)) {
          break;
        }
        auto next = it;
        std::advance(next, 1);
        if (next != mValues.rend()) {
          if (*it<value && * next> value) { // space is sorted and descending
            paramIndex = i;
            break;
          } else if (*it > value &&
                     *next < value) { // space is sorted and ascending
            paramIndex = i - 1;
            break;
          }
        }
        i--;
      }
    }
  }
  if (paramIndex < 0) {
    //          std::cerr << "WARNING: index not found" << std::endl;
    paramIndex = 0;
  }
  return paramIndex;
}

size_t ParameterSpaceDimension::getFirstIndexForId(std::string id,
                                                   bool reverse) {
  size_t paramIndex = std::numeric_limits<size_t>::max();

  if (!reverse) {
    size_t i = 0;
    for (auto it = mIds.begin(); it != mIds.end(); it++) {
      if (*it == id) {
        paramIndex = i;
        break;
      }
      i++;
    }
  } else {
    size_t i = mIds.size() - 1;
    for (auto it = mIds.rbegin(); it != mIds.rend(); it++) {
      if (*it == id) {
        paramIndex = i;
        break;
      }
      i--;
    }
  }
  return paramIndex;
}

float ParameterSpaceDimension::getCurrentValue() {
  if (mValues.size() > 0) {
    return mValues[getCurrentIndex()];
  } else {
    return mParameterValue.get();
  }
}

void ParameterSpaceDimension::setCurrentValue(float value) {
  parameter().set(value);
}

size_t ParameterSpaceDimension::getCurrentIndex() {
  return getIndexForValue(mParameterValue.get());
}

void ParameterSpaceDimension::setCurrentIndex(size_t index) {

  parameter().set(at(index));
}

std::string ParameterSpaceDimension::getCurrentId() {
  return idAt(getCurrentIndex());
}

std::string ParameterSpaceDimension::getName() { return parameter().getName(); }

std::string ParameterSpaceDimension::getGroup() {
  return parameter().getGroup();
}

std::string ParameterSpaceDimension::getFullAddress() {
  return parameter().getFullAddress();
}

std::vector<std::string> ParameterSpaceDimension::getAllCurrentIds() {
  float value = getCurrentValue();
  return getAllIds(value);
}

std::vector<std::string> ParameterSpaceDimension::getAllIds(float value) {
  size_t lowIndex = getFirstIndexForValue(value);
  size_t highIndex = getFirstIndexForValue(
      value, true); // Open range value (excluded from range)
  std::vector<std::string> ids;

  for (size_t i = lowIndex; i < highIndex; i++) {
    ids.push_back(idAt(i));
  }
  if (lowIndex ==
      highIndex) { // Hack... shouldn't need to be done. This should be fixed
                   // for one dimensional data parameter spaces.
    ids.push_back(idAt(lowIndex));
  }
  return ids;
}

std::vector<size_t> ParameterSpaceDimension::getAllCurrentIndeces() {
  float value = getCurrentValue();
  return getAllIndeces(value);
}

std::vector<size_t> ParameterSpaceDimension::getAllIndeces(float value) {
  size_t lowIndex = getFirstIndexForValue(value);
  size_t highIndex = getFirstIndexForValue(
      value, true); // Open range value (excluded from range)
  std::vector<size_t> idxs;

  for (size_t i = lowIndex; i < highIndex; i++) {
    idxs.push_back(i);
  }
  if (lowIndex ==
      highIndex) { // Hack... shouldn't need to be done. This should be fixed
                   // for one dimensional data parameter spaces.
    idxs.push_back(lowIndex);
  }
  return idxs;
}

al::Parameter &ParameterSpaceDimension::parameter() { return mParameterValue; }

void ParameterSpaceDimension::stepIncrement() {
  size_t curIndex = getCurrentIndex();
  float temp = mParameterValue.get();
  if (int64_t(curIndex) <
      int64_t(mValues.size()) -
          1) { // Check if we have an element above to compare to
    size_t offset = 1;
    float nextTemp = at(curIndex + offset);
    while (nextTemp == temp && curIndex < mValues.size() - ++offset) {
      nextTemp = at(curIndex + offset);
    }
    if (nextTemp >
        temp) { // Element above is greater, so increment index and load
      parameter().set(nextTemp);
    } else if (curIndex > 0 &&
               nextTemp !=
                   temp) { // If element above is not greater and we have space
                           // to go down, then decrement index and load
      float prevTemp = at(curIndex - 1);
      parameter().set(prevTemp);
    }
  } else if (curIndex > 0) { // Can we move one below?
    float prevTemp = at(curIndex - 1);
    if (prevTemp > temp) {
      parameter().set(prevTemp);
    } /*else if (curIndex = )*/
  }
}

void ParameterSpaceDimension::stepDecrease() {
  size_t curIndex = getCurrentIndex();
  float temp = mParameterValue.get();
  float nextTemp = temp;
  while (nextTemp == temp && (int)curIndex < (int)mValues.size() - 1) {
    nextTemp = at(++curIndex);
  }
  if (nextTemp <
      temp) { // Element above is greater, so increment index and load
    parameter().set(nextTemp);
  } else {
    float prevTemp = temp;
    while (prevTemp == temp && (int)curIndex > 0) {
      prevTemp = at(--curIndex);
    }
    if (prevTemp != temp) {
      parameter().set(prevTemp);
    }
  }
}

void ParameterSpaceDimension::push_back(float value, std::string id) {

  // FIXME There is no check to see if value is already present. Could cause
  // trouble
  // if value is there already.
  mValues.emplace_back(value);

  if (id.size() > 0) {
    mIds.push_back(id);
    if (mIds.size() != mValues.size()) {
      std::cerr
          << " ERROR! value and id mismatch in parameter space! This is bad."
          << std::endl;
    }
  }

  if (value > mParameterValue.max()) {
    mParameterValue.max(value);
  }
  if (value < mParameterValue.min()) {
    mParameterValue.min(value);
  }

  onDimensionMetadataChange(this);
}

void ParameterSpaceDimension::append(float *values, size_t count,
                                     std::string idprefix) {
  size_t oldSize = mValues.size();
  mValues.resize(mValues.size() + count);
  auto valueIt = mValues.begin() + oldSize;
  bool useIds = false;
  if (mIds.size() > 0 || idprefix.size() > 0) {
    useIds = true;
    mIds.resize(mValues.size() + count);
  }
  auto idIt = mIds.begin() + oldSize;
  for (size_t i = 0; i < count; i++) {
    if (useIds) {
      *idIt = idprefix + std::to_string(*values);
      idIt++;
    }
    *valueIt = *values;
    values++;
    valueIt++;
  }
  onDimensionMetadataChange(this);
}

void ParameterSpaceDimension::append(int32_t *values, size_t count,
                                     std::string idprefix) {
  size_t oldSize = mValues.size();
  mValues.resize(mValues.size() + count);
  auto valueIt = mValues.begin() + oldSize;
  auto idIt = mIds.begin() + oldSize;
  bool useIds = false;
  if (mIds.size() > 0 || idprefix.size() > 0) {
    useIds = true;
  }
  for (size_t i = 0; i < count; i++) {
    if (useIds) {
      *idIt = idprefix + std::to_string(*values);
      idIt++;
    }
    *valueIt = *values;
    values++;
    valueIt++;
  }
  onDimensionMetadataChange(this);
}

void ParameterSpaceDimension::append(uint32_t *values, size_t count,
                                     std::string idprefix) {
  size_t oldSize = mValues.size();
  mValues.resize(mValues.size() + count);
  auto valueIt = mValues.begin() + oldSize;
  auto idIt = mIds.begin() + oldSize;
  bool useIds = false;
  if (mIds.size() > 0 || idprefix.size() > 0) {
    useIds = true;
  }
  for (size_t i = 0; i < count; i++) {
    if (useIds) {
      *idIt = idprefix + std::to_string(*values);
      idIt++;
    }
    *valueIt = *values;
    values++;
    valueIt++;
  }
  onDimensionMetadataChange(this);
}

void ParameterSpaceDimension::append(uint8_t *values, size_t count,
                                     std::string idprefix) {
  size_t oldSize = mValues.size();
  mValues.resize(mValues.size() + count);
  bool useIds = false;
  if (mIds.size() > 0 || idprefix.size() > 0) {
    useIds = true;
    mIds.resize(mIds.size() + count);
  }
  auto valueIt = mValues.begin() + oldSize;
  auto idIt = mIds.begin() + oldSize;
  for (size_t i = 0; i < count; i++) {
    if (useIds) {
      *idIt = idprefix + std::to_string(*values);
      idIt++;
    }
    *valueIt = *values;
    values++;
    valueIt++;
  }
  onDimensionMetadataChange(this);
}

void ParameterSpaceDimension::conform() {
  mParameterValue.max(std::numeric_limits<float>::min());
  mParameterValue.min(std::numeric_limits<float>::max());
  for (auto value : mValues) {

    if (value > mParameterValue.max()) {
      mParameterValue.max(value);
    }
    if (value < mParameterValue.min()) {
      mParameterValue.min(value);
    }
  }
  mParameterValue.setNoCalls(at(0));
}

void ParameterSpaceDimension::reserve(size_t totalSize) {
  mValues.reserve(totalSize);
}

void ParameterSpaceDimension::addConnectedParameterSpace(
    ParameterSpaceDimension *paramSpace) {
  mConnectedSpaces.push_back(paramSpace);
}

std::shared_ptr<ParameterSpaceDimension> ParameterSpaceDimension::deepCopy() {
  lock();
  auto dimCopy =
      std::make_shared<ParameterSpaceDimension>(getName(), getGroup());
  dimCopy->datatype = datatype;
  dimCopy->mRepresentationType = mRepresentationType;
  dimCopy->mIds = mIds;
  dimCopy->mValues = mValues;
  dimCopy->setCurrentIndex(getCurrentIndex());

  unlock();
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

void ParameterSpaceDimension::sort() {
  lock();

  std::vector<size_t> indeces;
  indeces.resize(mValues.size());
  for (size_t i = 0; i < indeces.size(); i++) {
    indeces[i] = i;
  }
  std::sort(indeces.begin(), indeces.end(), sort_indices(mValues.data()));
  std::vector<float> newValues;
  std::vector<std::string> newIds;
  newValues.reserve(indeces.size());
  newIds.reserve(indeces.size());
  if (mValues.size() != mIds.size() && mIds.size() > 0) {
    std::cerr << "ERROR: sort() will crash (or lead ot unexpected behavior) as "
                 "the size of values and ids don't match."
              << std::endl;
  }
  for (size_t i = 0; i < indeces.size(); i++) {
    size_t index = indeces[i];
    newValues.push_back(mValues[index]);
    if (mIds.size() > 0) {
      newIds.push_back(mIds[index]);
    }
  }
  mValues = newValues;
  mIds = newIds;
  unlock();
  onDimensionMetadataChange(this);
}

std::vector<float> ParameterSpaceDimension::values() { return mValues; }

std::vector<std::string> ParameterSpaceDimension::ids() { return mIds; }
