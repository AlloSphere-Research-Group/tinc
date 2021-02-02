#ifndef VARIANTVALUE_HPP
#define VARIANTVALUE_HPP

/*
 * Copyright 2021 AlloSphere Research Group
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

#include <string>
#include <cinttypes>

namespace tinc {
enum VariantType {
  VARIANT_INT64 = 0,
  VARIANT_INT32,
  VARIANT_DOUBLE,
  VARIANT_FLOAT,
  VARIANT_STRING,
  VARIANT_NULL
};

struct VariantValue {
  VariantValue() { type = VARIANT_NULL; }

  VariantValue(std::string value) {
    type = VARIANT_STRING;
    valueStr = value;
  }
  VariantValue(const char *value) {
    type = VARIANT_STRING;
    valueStr = value;
  }

  VariantValue(int64_t value) {
    type = VARIANT_INT64;
    valueInt64 = value;
  }

  VariantValue(int32_t value) {
    type = VARIANT_INT32;
    valueInt64 = value;
  }

  VariantValue(double value) {
    type = VARIANT_DOUBLE;
    valueDouble = value;
  }

  VariantValue(float value) {
    type = VARIANT_FLOAT;
    valueDouble = value;
  }

  //  ~VariantValue()
  //  {
  //      delete[] cstring;  // deallocate
  //  }

  //  VariantValue(const VariantValue& other) // copy constructor
  //      : VariantValue(other.cstring)
  //  {}

  //  VariantValue(VariantValue&& other) noexcept // move constructor
  //      : cstring(std::exchange(other.cstring, nullptr))
  //  {}

  //  VariantValue& operator=(const VariantValue& other) // copy assignment
  //  {
  //      return *this = VariantValue(other);
  //  }

  //  VariantValue& operator=(VariantValue&& other) noexcept // move assignment
  //  {
  //      std::swap(cstring, other.cstring);
  //      return *this;
  //  }

  std::string commandFlag; // A prefix to the flag (e.g. -o)

  VariantType type;
  std::string valueStr;
  int64_t valueInt64;
  double valueDouble;
};
}

#endif // VARIANTVALUE_HPP
