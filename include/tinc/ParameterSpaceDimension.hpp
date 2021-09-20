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

// This enum must be kept in sync with the one in protobuf
enum ParameterType {
  PARAMETER_FLOAT = 0,
  PARAMETER_BOOL = 1,
  PARAMETER_STRING = 2,
  PARAMETER_INT32 = 3,
  PARAMETER_VEC3F = 4,
  PARAMETER_VEC4F = 5,
  PARAMETER_COLORF = 6,
  PARAMETER_POSED = 7,
  PARAMETER_MENU = 8,
  PARAMETER_CHOICE = 9,
  PARAMETER_TRIGGER = 10,

  PARAMETER_INT64 = 11,
  PARAMETER_INT16 = 12,
  PARAMETER_INT8 = 13,
  PARAMETER_UINT64 = 14,
  PARAMETER_UINT32 = 15,
  PARAMETER_UINT16 = 16,
  PARAMETER_UINT8 = 17,
  PARAMETER_DOUBLE = 18,
};

/**
 * @brief Class to represent a dimension for a ParameterSpace
 *
 * This class provides data storage, interactivity and (optionally)
 * discretization for the dimension.
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
  typedef enum { VALUE = 0x00, INDEX = 0x01, ID = 0x02 } RepresentationType;

  /**
   * @brief ParameterSpaceDimension
   * @param name
   * @param group
   * @param paramType
   *
   * Dimensions must have names and can belong to groups.
   * paramType determines both data type as well as interface and
   * behavior.
   */
  ParameterSpaceDimension(
      std::string name, std::string group = "",
      ParameterType paramType = ParameterType::PARAMETER_FLOAT);
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
   * Set current value from float
   */
  bool setCurrentValue(float value);

  /**
   * Get current value as a float
   */
  float getCurrentValue();

  /**
   * @brief Set current index
   */
  void setCurrentIndex(size_t index);

  /**
   * @brief get index of current value in parameter space
   * @return index
   */
  size_t getCurrentIndex();

  /**
   * @brief get all index for current value in parameter space
   * @return vector of indeces
   *
   * If the parameter space has been defined so that one value can be mapped to
   * multiple ids, this function will return all indeces at which the current
   * value occurs.
   */
  std::vector<size_t> getCurrentIndeces();

  /**
   * @brief get current id
   * @return id
   *
   * If there are no ids defined, or the current value is invalid, an empty
   * string is returned.
   */
  std::string getCurrentId();

  /**
   * @brief getCurrentIds
   * @return
   *
   * See getCurrentIndeces()
   */
  std::vector<std::string> getCurrentIds();

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
    if (mRepresentationType != type) {
      mRepresentationType = type;
      onDimensionMetadataChange(this, src);
    }
  }

  /**
   * Get current representation type
   */
  RepresentationType getSpaceRepresentationType() {
    return mRepresentationType;
  }
  /**
   * @brief get number of elements to jump to move to next value
   * @return
   */
  size_t getSpaceStride() { return mSpaceValues->stride(); }

  /**
   * The parameter instance holds the current value.
   * You can set values for the internal parameter through this function,
   * register notifications and create GUIs/ network synchronization.
   */
  template <typename ParameterType> ParameterType &getParameter() {
    return *static_cast<ParameterType *>(mParameterValue);
  }
  /**
   * @brief get abstract ParameterMeta pointer to internal parameter
   * @return
   *
   * To determine the type of parameter, you can test if dynamic_cast<>() !=
   * nullptr.
   */
  al::ParameterMeta *getParameterMeta() { return mParameterValue; }

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

  /**
   * @brief sort
   * @param src socket that generated this request
   */
  void sort(al::Socket *src = nullptr);

  /**
   * @brief Clear the parameter space
   * @param src socket that generated this request
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

  /**
   * @brief get all ids for a value
   * @param value
   * @return ids
   */
  std::vector<std::string> getIdsForValue(float value);

  // Discrete parameter space values
  /**
   * @brief Set possible values in the parameter space dimension
   * @param values pointer to array of values
   * @param count number of values in pointer
   * @param idprefix prefix for auto generated ids
   * @param src source socket if this function is called from the network
   */
  void setSpaceValues(void *values, size_t count, std::string idprefix = "",
                      al::Socket *src = nullptr);

  /**
   * @brief Set possible values in the parameter space dimension
   * @param values pointer to array of values
   * @param idprefix prefix for auto generated ids
   * @param src source socket if this function is called from the network
   *
   * You will need to call conformSpace() to ensure value and minimum and
   * maximum are update to reflect the parameter space.
   */
  template <typename SpaceDataType>
  void setSpaceValues(std::vector<SpaceDataType> values,
                      std::string idprefix = "", al::Socket *src = nullptr) {
    mSpaceValues->clear();
    // TODO add safety check for types and pointer sizes
    mSpaceValues->append(values.data(), values.size(), idprefix);
    onDimensionMetadataChange(this, src);
  }
  /**
   * @brief append values to current dimension values
   * @param values pointer to array of values
   * @param count number of values in pointer
   * @param idprefix prefix for auto generated ids
   * @param src source socket if this function is called from the network
   */
  void appendSpaceValues(void *values, size_t count, std::string idprefix = "",
                         al::Socket *src = nullptr);

  /**
   * Get possible values this dimension can take
   */
  template <typename SpaceDataType>
  std::vector<SpaceDataType> getSpaceValues() {
    return mSpaceValues->getValues<SpaceDataType>();
  }

  /**
   * Set ids for space values. You must ensure that the size of the space and
   * the number of ids matches.
   */
  void setSpaceIds(std::vector<std::string> ids, al::Socket *src = nullptr);

  /**
   * Get vector containing all space ids
   */
  std::vector<std::string> getSpaceIds();

  /**
   * @brief Get Data type for space data.
   *
   * The space data might be different to the parameter data type. Spaces in
   * int8, int16 and in32 are all handled by ParameterInt that uses a 32 bit
   * representation.
   */
  al::VariantType getSpaceDataType() { return mSpaceValues->getDataType(); }

  /**
   * @brief Get index in space for value
   * @param value
   * @return the index
   */
  size_t getIndexForValue(float value);

  /**
   * @brief Adjust range according to current values in parameter space
   * @return true if conform was succesful and space is consistent
   *
   * The minimum and maximum value are stored separately from the values
   * the parameter can take, so you must set them manually or use this function.
   */
  bool conformSpace(al::Socket *src = nullptr);

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

  void print(std::ostream &stream = std::cout);

  /**
   * This function is called whenever dimension metadata cahnges, to notify
   * connected clients. 'src' provides the socket that originated the change, to
   * avoid resending the change to that socket.
   */
  std::function<void(ParameterSpaceDimension *, al::Socket *src)>
      onDimensionMetadataChange =
          [](ParameterSpaceDimension *, al::Socket *src) {};

private:
  // Used to store discretization values of parameters
  std::unique_ptr<al::DiscreteParameterValues> mSpaceValues;

  RepresentationType mRepresentationType{VALUE};
  bool mFilesystemDimension{false};

  // Current state
  al::ParameterMeta *mParameterValue{nullptr};
  ParameterType mParamType;
  bool mParamInternal;
};

} // namespace tinc

#endif // PARAMETERSPACEDIMENSION_HPP
