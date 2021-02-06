#include "tinc/DataPoolJson.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <fstream>

using namespace tinc;

bool DataPoolJson::getFieldFromFile(std::string field, std::string file,
                                    size_t dimensionInFileIndex, void *data) {
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return false;
  }
  json j = json::parse(f);
  auto fieldData = j[field].at(dimensionInFileIndex).get<float>();
  *(float *)data = fieldData;
  return true;
}

bool DataPoolJson::getFieldFromFile(std::string field, std::string file,
                                    void *data, size_t length) {

  auto fileType = getFileType(file);
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return false;
  }
  json j;
  f >> j;
  auto fieldData = j[field];
  if (!fieldData.is_null() && fieldData.is_array()) {
    memcpy((float *)data, fieldData.get<std::vector<float>>().data(),
           length * sizeof(float));
  } else {
    return false;
  }
  return true;
}
