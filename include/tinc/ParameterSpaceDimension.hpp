#ifndef PARAMETERSPACEDIMENSION_HPP
#define PARAMETERSPACEDIMENSION_HPP

/*
 * Copyright 2020 AlloSphere Research Group
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 *        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * authors: Andres Cabrera
*/

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
  using Datatype = al::DiscreteParameterValues::Datatype;
  typedef enum { VALUE = 0x00, INDEX = 0x01, ID = 0x02 } RepresentationType;

  ParameterSpaceDimension(std::string name, std::string group = "",
                          Datatype dataType = Datatype::FLOAT);

  //
  ParameterSpaceDimension(al::ParameterMeta *param);

  ~ParameterSpaceDimension();

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
    return *static_cast<ParameterType *>(mParameterValue);
  }

  al::ParameterMeta *parameterMeta() { return mParameterValue; }

  // Move current position in parameter space
  void stepIncrement();
  void stepDecrease();

  size_t size();

  //  void sort();
  void clear();

  float at(size_t index);
  std::string idAt(size_t index);

  // Discrete parameter space values
  void setSpaceValues(void *values, size_t count, std::string idprefix = "",
                      bool propagate = true);
  void setSpaceValues(std::vector<float> values, std::string idprefix = "",
                      bool propagate = true);
  void appendSpaceValues(void *values, size_t count, std::string idprefix = "",
                         bool propagate = true);

  template <typename SpaceDataType>
  std::vector<SpaceDataType> getSpaceValues() {
    return mSpaceValues.getValues<SpaceDataType>();
  }

  void setSpaceIds(std::vector<std::string> ids, bool propagate = true);

  std::vector<std::string> getSpaceIds();

  al::DiscreteParameterValues::Datatype getSpaceDataType() {
    return mSpaceValues.getDataType();
  }

  size_t getIndexForValue(float value);

  // Set limits from internal data and sort
  void conformSpace();

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
  al::ParameterMeta *mParameterValue{nullptr};
  bool mParamInternal;
};

} // namespace tinc

#endif // PARAMETERSPACEDIMENSION_HPP
