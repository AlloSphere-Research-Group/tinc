#include "tinc/DataPoolJson.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <fstream>

using namespace tinc;

std::vector<std::string> DataPoolJson::listFieldInFile(std::string file) {
  std::vector<std::string> fields;
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return {};
  }
  json j = json::parse(f);
  if (j.is_object()) {
    for (auto entry = j.begin(); entry != j.end(); entry++) {
      fields.push_back(entry.key());
    }
  }
  return fields;
}

bool DataPoolJson::getFieldFromFile(std::string field, std::string file,
                                    size_t dimensionInFileIndex, void *data) {
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return false;
  }
  json j = json::parse(f);
  if (j.is_object() && j.contains(field)) {
    auto fieldData = j[field].at(dimensionInFileIndex).get<double>();
    *(double *)data = fieldData;
    return true;
  } else {
    return false;
  }
}

bool DataPoolJson::getFieldFromFile(std::string field, std::string file,
                                    void *data, size_t length) {
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return false;
  }
  json j;
  f >> j;
  auto fieldData = j[field];
  if (!fieldData.is_null() && fieldData.is_array()) {
    memcpy((double *)data, fieldData.get<std::vector<double>>().data(),
           length * sizeof(double));
  } else {
    return false;
  }
  return true;
}
