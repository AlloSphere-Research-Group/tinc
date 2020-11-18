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
#include "al/ui/al_DiscreteParameterValues.hpp"

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

  using Datatype = al::DiscreteParameterValues::Datatype;
  typedef enum { VALUE = 0x00, INDEX = 0x01, ID = 0x02 } RepresentationType;

  ParameterSpaceDimension(std::string name, std::string group = "",
                          Datatype dataType = Datatype::FLOAT);
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
  // TODO this member should be removed. This should be handled by
  // ParameterSpace instead
  bool isFilesystemDimension() { return mFilesystemDimension; }

  void setFilesystemDimension(bool set = true) {
    mFilesystemDimension = set;
    onDimensionMetadataChange(this);
  }

  // the parameter instance holds the current value.
  // You can set values for parameter space through this function
  // Register notifications and create GUIs/ network synchronization
  // Through this instance.
  template <typename ParameterType> ParameterType &parameter() {
    return *static_cast<ParameterType *>(mParameterValue.get());
  }

  al::ParameterMeta *parameterMeta() { return mParameterValue.get(); }

  // Move current position in parameter space
  void stepIncrement();
  void stepDecrease();

  size_t size();

  //  void sort();
  void clear();

  float at(size_t index);
  std::string idAt(size_t index);

  // Discrete parameter space values
  void setSpaceValues(void *values, size_t count, std::string idprefix = "");
  void setSpaceValues(std::vector<float> values, std::string idprefix = "");
  void appendSpaceValues(void *values, size_t count, std::string idprefix = "");

  template <typename SpaceDataType>
  std::vector<SpaceDataType> getSpaceValues() {
    return mSpaceValues.getValues<SpaceDataType>();
  }

  void setSpaceIds(std::vector<std::string> ids);

  std::vector<std::string> getSpaceIds();

  al::DiscreteParameterValues::Datatype getSpaceDataType() {
    return mSpaceValues.getDataType();
  }

  size_t getIndexForValue(float value);

  // Set limits from internal data
  void conform();

  void addConnectedParameterSpace(ParameterSpaceDimension *paramSpace);

  std::shared_ptr<ParameterSpaceDimension> deepCopy();

  std::function<void(ParameterSpaceDimension *)> onDimensionMetadataChange = [](
      ParameterSpaceDimension *) {};

private:
  // Data
  //  std::vector<float> mValues;
  //  std::vector<std::string> mIds;

  // Used to store discretization values of parameters
  al::DiscreteParameterValues mSpaceValues;

  RepresentationType mRepresentationType{VALUE};
  bool mFilesystemDimension{false};

  // Current state
  std::unique_ptr<al::ParameterMeta> mParameterValue;
};

} // namespace tinc

#endif // PARAMETERSPACEDIMENSION_HPP
