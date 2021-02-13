#include "tinc/VariantValue.hpp"

using namespace tinc;

VariantValue::VariantValue(std::string value) {
  type = VARIANT_STRING;
  valueStr = value;
}

VariantValue::VariantValue(const char *value) {
  type = VARIANT_STRING;
  valueStr = value;
}

VariantValue::VariantValue(int64_t value) {
  type = VARIANT_INT64;
  valueInt64 = value;
}

VariantValue::VariantValue(int32_t value) {
  type = VARIANT_INT32;
  valueInt64 = value;
}

VariantValue::VariantValue(int8_t value) {
  type = VARIANT_INT8;
  valueInt64 = value;
}

VariantValue::VariantValue(uint64_t value) {
  type = VARIANT_UINT64;
  valueUInt64 = value;
}

VariantValue::VariantValue(uint32_t value) {
  type = VARIANT_UINT32;
  valueUInt64 = value;
}

VariantValue::VariantValue(uint8_t value) {
  type = VARIANT_UINT8;
  valueUInt64 = value;
}

VariantValue::VariantValue(double value) {
  type = VARIANT_DOUBLE;
  valueDouble = value;
}

VariantValue::VariantValue(float value) {
  type = VARIANT_FLOAT;
  valueDouble = value;
}
