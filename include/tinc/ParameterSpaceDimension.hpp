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

#include "al/io/al_Socket.hpp"
#include "al/ui/al_DiscreteParameterValues.hpp"
#include "al/ui/al_Parameter.hpp"

namespace tinc {

/**
 * @brief The ParameterSpaceDimension class provides a discrete dimension with
 *possible values
 *
 * It allows mapping a discrete set of parameter values to string ids, for
 * example for mapping values to filesystem names.
 * A ParameterSpaceDimension groups the possible values or states parameters can
 * take. It also holds a "current" value in this parameter space.
 *
 *In NetCDF parlance, a ParameterSpaceDimension encapsulates both a variable and
 *a dimension. As it deals both with the shape and the values of the parameter
 *space.
 */
class ParameterSpaceDimension {
  friend class ParameterSpace;
  friend class TincProtocol;

public:
  using Datatype = al::DiscreteParameterValues::Datatype;
  typedef enum { VALUE = 0x00, INDEX = 0x01, ID = 0x02 } RepresentationType;

  /**
   * @brief ParameterSpaceDimension
   * @param name
   * @param group
   * @param dataType
   *
   * Dimensions can have names and belong to groups.
   */
  ParameterSpaceDimension(std::string name, std::string group = "",
                          Datatype dataType = Datatype::FLOAT);
  /**
   * @brief construct a ParameterSpaceDimension from a ParameterMeta *
   * @param param
   * @param makeInternal
   */
  ParameterSpaceDimension(al::ParameterMeta *param, bool makeInternal = false);

  ~ParameterSpaceDimension();

  // disallow copy constructor
  ParameterSpaceDimension(const ParameterSpaceDimension &other) = delete;
  // disallow copy assignment
  ParameterSpaceDimension &
  operator=(const ParameterSpaceDimension &other) = delete;

  /**
   * @brief Get the name of the dimension
   * @return the name
   */
  std::string getName();

  /**
   * @brief returns the dimension's group
   * @return group name
   */
  std::string getGroup();

  /**
   * @brief Get OSC address for dimension
   * @return OSC address
   *
   * OSC address joins group and parameter name as: /group/name
   * This serves as a unique identifier for dimensions within a parameter space
   */
  std::string getFullAddress();

  // ---- Data access
  /**
   * Get current value as a float
   */
  float getCurrentValue();
  /**
   * Set current value from float
   */
  void setCurrentValue(float value);

  /**
   * @brief get index of current value in parameter space
   * @return index
   */
  size_t getCurrentIndex();
  /**
   * @brief Set current index
   */
  void setCurrentIndex(size_t index);

  /**
   * @brief get current id
   * @return id
   *
   * If there are no ids defined, or the current value is invalid, an empty
   * string is returned.
   */
  std::string getCurrentId();

  /**
   * @brief set representation type for dimension.
   * @param type
   * @param src
   *
   * This determines the preferred representation for the dimension, for example
   * in gui objects.
   */
  void setSpaceRepresentationType(RepresentationType type,
                                  al::Socket *src = nullptr) {
    mRepresentationType = type;
    onDimensionMetadataChange(this, src);
  }

  /**
   * Get current representation type
   */
  RepresentationType getSpaceRepresentationType() {
    return mRepresentationType;
  }

  /**
   * The parameter instance holds the current value.
   * You can set values for the internal parameter through this function,
   * register notifications and create GUIs/ network synchronization.
   */
  template <typename ParameterType> ParameterType &parameter() {
    return *static_cast<ParameterType *>(mParameterValue);
  }

  al::ParameterMeta *parameterMeta() { return mParameterValue; }

  /**
   * Step to the nearest index that increments the paramter value. This could
   * result in an increase or decrease of the index.
   * Requires that the space is sorted in ascending or descending order.
   */
  void stepIncrement();
  /**
   * Step to the nearest index that decrements the paramter value. This could
   * result in an increase or decrease of the index.
   * Requires that the space is sorted in ascending or descending order.
   */
  void stepDecrease();

  /**
   *  Size of the defined values in the parameter space
   */
  size_t size();

  void sort();

  /**
   * @brief Clear the parameter space
   * @param src
   */
  void clear(al::Socket *src = nullptr);

  /**
   * Get value at index 'index' as float
   */
  float at(size_t index);

  /**
   * Get id at index
   */
  std::string idAt(size_t index);

  // Discrete parameter space values
  // FIXME improve these setters
  void setSpaceValues(void *values, size_t count, std::string idprefix = "",
                      al::Socket *src = nullptr);
  void setSpaceValues(std::vector<float> values, std::string idprefix = "",
                      al::Socket *src = nullptr);
  void appendSpaceValues(void *values, size_t count, std::string idprefix = "",
                         al::Socket *src = nullptr);

  template <typename SpaceDataType>
  std::vector<SpaceDataType> getSpaceValues() {
    return mSpaceValues.getValues<SpaceDataType>();
  }

  void setSpaceIds(std::vector<std::string> ids, al::Socket *src = nullptr);

  std::vector<std::string> getSpaceIds();

  al::DiscreteParameterValues::Datatype getSpaceDataType() {
    return mSpaceValues.getDataType();
  }

  size_t getIndexForValue(float value);

  /**
   * @brief Adjust range according to current values in parameter space
   *
   * The minimum and maximum value are stored separately from the values
   * the parameter can take, so you must set them manually or use this function.
   */
  void conformSpace();

  /**
   * @brief provide a deep copy of the parameter space
   * @return the copy
   *
   * This is useful when you need to capture the state of a
   * ParameterSpaceDimension.
   *
   * Note that currently callbacks for parameters are not being copied.
   */
  std::shared_ptr<ParameterSpaceDimension> deepCopy();

  /**
   * This function is called whenever dimension metadata cahnges, to notify
   * connected clients. 'src' provides the socket that originated the change, to
   * avoid resending the change to that socket.
   */
  std::function<void(ParameterSpaceDimension *, al::Socket *src)>
      onDimensionMetadataChange = [](ParameterSpaceDimension *,
                                     al::Socket *src) {};

private:
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
