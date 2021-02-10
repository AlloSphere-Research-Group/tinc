#include "tinc/DataPoolNetCDF.hpp"

#ifdef TINC_HAS_NETCDF
#include <netcdf.h>
#endif

#include <fstream>

using namespace tinc;

bool DataPool::getFieldFromFile(std::string field, std::string file,
                                size_t dimensionInFileIndex, void *data) {
  // FIXME implement
  //  std::ifstream f(file);
  //  if (!f.good()) {
  //    std::cerr << "ERROR reading file: " << file << std::endl;
  //    return false;
  //  }
  //  json j = json::parse(f);
  //  auto fieldData = j[field].at(dimensionInFileIndex).get<float>();
  //  *(float *)data = fieldData;
  return true;
}

bool DataPool::getFieldFromFile(std::string field, std::string file, void *data,
                                size_t length) {

  // FIXME implement
  //  auto fileType = getFileType(file);
  //  std::ifstream f(file);
  //  if (!f.good()) {
  //    std::cerr << "ERROR reading file: " << file << std::endl;
  //    return false;
  //  }
  //  json j;
  //  f >> j;
  //  auto fieldData = j[field];
  //  if (fieldData && fieldData.is_array()) {
  //    memcpy((float *)data, fieldData.get<std::vector<float>>().data(),
  //           length * sizeof(float));
  //  } else {
  //    return false;
  //  }
  return true;
}

std::vector<std::string> DataPoolNetCDF::listFieldInFile(std::string file) {
  std::vector<std::string> fields;
  // FIXME implement
  return fields;
}
