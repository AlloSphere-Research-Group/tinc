#ifndef PARAMETERSPACEDIMENSION_HPP
#define PARAMETERSPACEDIMENSION_HPP

#ifdef AL_WINDOWS
#define NOMINMAX
#include <Windows.h>
#undef far
#endif

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "al/ui/al_Parameter.hpp"

namespace tinc {

/**
 * @brief The ParameterSpaceDimension class maps parameter values to string ids
 *
 * This allows mapping continuous parameters to string ids, for example
 * for mapping to directory structures
 * A parameter space groups discrete dimensions together that represent the
 * possible values or states parameters can take. The paramter space class also
 * holds a "current" value in this parameter space.
 *
 */
class ParameterSpaceDimension {
  friend class ParameterSpace;

public:
  // TODO implement more data types (in particular DOUBLE, INT64, STRING)
  typedef enum { FLOAT, UINT8, INT32, UINT32 } Datatype;

  typedef enum { VALUE = 0x00, INDEX = 0x01, ID = 0x02 } RepresentationType;

  ParameterSpaceDimension(std::string name, std::string group = "")
      : mParameterValue(name, group) {}
  std::string getName();
  std::string getGroup();
  std::string getFullAddress();

  // Access to current
  float getCurrentValue();
  void setCurrentValue(float value);

  size_t getCurrentIndex();
  void setCurrentIndex(size_t index);
  std::string getCurrentId();

  void setSpaceRepresentationType(RepresentationType type) {
    mRepresentationType = type;
    onDimensionMetadataChange(this);
  }

  RepresentationType getSpaceRepresentationType() {
    return mRepresentationType;
  }

  // This dimension affects the filesystem. All filesystem dimension in a
  // parameter space must have the smae size
  bool isFilesystemDimension() { return mFilesystemDimension; }

  void setFilesystemDimension(bool set = true) {
    mFilesystemDimension = set;
    onDimensionMetadataChange(this);
  }

  // Multidimensional parameter spaces will result in single values having
  // multiple ids. This can be resolved externally using this function
  // but perhaps we should have a higher level class that solves this issue for
  // the general case
  std::vector<std::string> getAllCurrentIds();
  std::vector<size_t> getAllCurrentIndeces();

  // Access to specific elements

  // When using reverse = true, the value returned describes the
  // end of an open range, i.e, the index returned is one more than
  // an index that would match the value.
  size_t getFirstIndexForValue(float value, bool reverse = false);

  /**
   * @brief getFirstIndexForId
   * @param id
   * @param reverse
   * @return
   *
   * Returns numeric_limits<size_t>::max() if id is not found.
   */
  size_t getFirstIndexForId(std::string id, bool reverse = false);

  float at(size_t x);

  std::string idAt(size_t x);
  size_t getIndexForValue(float value);

  // FIXME Are these getAll still relevant?
  std::vector<std::string> getAllIds(float value);
  std::vector<size_t> getAllIndeces(float value);

  // Access to complete sets
  std::vector<float> values();
  std::vector<std::string> ids();

  // the parameter instance holds the current value.
  // You can set values for parameter space through this function
  // Register notifications and create GUIs/ network synchronization
  // Through this instance.
  al::Parameter &parameter();

  // Move current position in parameter space
  void stepIncrement();
  void stepDecrease();

  size_t size();
  void reserve(size_t totalSize);

  void sort();
  void clear();

  // This is very inefficient and should be avoided if possible
  void push_back(float value, std::string id = "");

  void append(float *values, size_t count, std::string idprefix = "");
  void append(int32_t *values, size_t count, std::string idprefix = "");
  void append(uint32_t *values, size_t count, std::string idprefix = "");
  void append(uint8_t *values, size_t count, std::string idprefix = "");

  // Set limits from internal data
  void conform();

  void addConnectedParameterSpace(ParameterSpaceDimension *paramSpace);

  // Protect parameter space (to avoid access during modification)
  // TODO all readers above need to use this lock
  void lock() { mLock.lock(); }
  void unlock() { mLock.unlock(); }

  // TODO these should be made private and access functions added that call
  // onDimensionMetadataChange()
  Datatype datatype{FLOAT};

  std::shared_ptr<ParameterSpaceDimension> deepCopy();

  std::function<void(ParameterSpaceDimension *)> onDimensionMetadataChange = [](
      ParameterSpaceDimension *) {};

private:
  // Data
  std::vector<float> mValues;
  std::vector<std::string> mIds;

  RepresentationType mRepresentationType{VALUE};
  bool mFilesystemDimension{false};

  std::mutex mLock;

  // Current state
  al::Parameter mParameterValue;

  std::vector<ParameterSpaceDimension *> mConnectedSpaces;
};

} // namespace tinc

#endif // PARAMETERSPACEDIMENSION_HPP
