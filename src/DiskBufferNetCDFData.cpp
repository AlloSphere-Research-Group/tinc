#include "tinc/DiskBufferNetCDFData.hpp"

using namespace tinc;

NetCDFData::~NetCDFData() { deleteData(); }

int NetCDFData::getType() { return ncDataType; }

void NetCDFData::setType(int type) {
  if (type == ncDataType) {
    return;
  }
  deleteData();
  switch (type) {
  case al::VariantType::VARIANT_INT16:
    dataVector = new std::vector<int16_t>;
    break;
  case al::VariantType::VARIANT_INT32:
    dataVector = new std::vector<int32_t>;
    break;
  case al::VariantType::VARIANT_FLOAT:
    dataVector = new std::vector<float>;
    break;
  case al::VariantType::VARIANT_DOUBLE:
    dataVector = new std::vector<double>;
    break;
  case al::VariantType::VARIANT_UINT8:
  case al::VariantType::VARIANT_CHAR:
    dataVector = new std::vector<uint8_t>;
    break;
  case al::VariantType::VARIANT_UINT16:
    dataVector = new std::vector<uint16_t>;
    break;
  case al::VariantType::VARIANT_UINT32:
    dataVector = new std::vector<int32_t>;
    break;
  case al::VariantType::VARIANT_INT64:
    dataVector = new std::vector<int64_t>;
    break;
  case al::VariantType::VARIANT_UINT64:
    dataVector = new std::vector<uint64_t>;
    break;
  case al::VariantType::VARIANT_STRING:
    dataVector = new std::vector<std::string>;

    std::cerr << "string not yet supported in DisnkBufferNetCDF" << std::endl;
    break;
  case al::VariantType::VARIANT_INT8:
    dataVector = new std::vector<int8_t>;

  case al::VariantType::VARIANT_NONE:
  default:
    ncDataType = al::VariantType::VARIANT_NONE;
    return;
  }

  ncDataType = (al::VariantType)type;
}

void NetCDFData::deleteData() {
  if (dataVector) {
    switch (ncDataType) {
    case SHORT:
      delete static_cast<std::vector<int16_t> *>(dataVector);
      break;
    case INT:
      delete static_cast<std::vector<int32_t> *>(dataVector);
      break;
    case FLOAT:
      delete static_cast<std::vector<float> *>(dataVector);
      break;
    case DOUBLE:
      delete static_cast<std::vector<double> *>(dataVector);
      break;
    case UBYTE:
    case CHAR:
      delete static_cast<std::vector<uint8_t> *>(dataVector);
      break;
    case USHORT:
      delete static_cast<std::vector<uint16_t> *>(dataVector);
      break;
    case UINT:
      delete static_cast<std::vector<int32_t> *>(dataVector);
      break;
    case INT64:
      delete static_cast<std::vector<int64_t> *>(dataVector);
      break;
    case UINT64:
      delete static_cast<std::vector<uint64_t> *>(dataVector);
      break;
    case STRING:
      delete static_cast<std::vector<std::string> *>(dataVector);
      std::cerr << "string not yet supported in DisnkBufferNetCDF" << std::endl;
      break;
    case BYTE:
      delete static_cast<std::vector<int8_t> *>(dataVector);
    }

    dataVector = nullptr;
  }
}
