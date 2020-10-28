#include "tinc/DataPool.hpp"
#include "tinc/CppProcessor.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <fstream>

int main() {

  // Create parameter space with two dimensions, one that affects the directory
  // and another that affects data within a file.
  tinc::ParameterSpace ps;

  auto dirDim =
      ps.newDimension("dirDim", tinc::ParameterSpaceDimension::ID);
  uint8_t values[] = {0, 2, 4, 6, 8};
  dirDim->append(values, 5, "datapool_directory_");

  auto internalValuesDim = ps.newDimension("internalValuesDim");
  float internalValues[] = {-0.3f, -0.2f, -0.1f, 0.0f, 0.1f, 0.2f, 0.3f};
  internalValuesDim->append(internalValues, 7);

  ps.generateRelativeRunPath = [](std::map<std::string, size_t> indeces,
                                  tinc::ParameterSpace *ps) {
    std::string path = ps->getDimension("dirDim")->idAt(indeces["dirDim"]);
    return path;
  };

  // Make sure our filesystem is fresh
  ps.cleanDataDirectories();

  // Make a simple processor to generate data in the parameter space
  tinc::CppProcessor dataCreator;
  dataCreator.processingFunction = [&]() {
    // Append values into the file
    json j;
    std::ifstream in("datapool_data.json");
    if (in.good()) {
      in >> j;
      in.close();
    }
    j["value"].push_back(
        dataCreator.configuration["internalValuesDim"].valueDouble *
        (1.0 + std::stoi(dataCreator.configuration["dirDim"].valueStr.substr(
                   sizeof("datapool_directory_") - 1))));

    std::ofstream out("datapool_data.json");
    if (out.good()) {
      out << j;
    }
    return true;
  };
  dataCreator.verbose(false);

  // Generate data
  ps.sweep(dataCreator);

  // Print data just for reference to understand slicing
  for (auto directory : ps.runningPaths()) {
    std::ifstream in(directory + "/datapool_data.json");
    std::cout << " ***** FILE: " << directory + "/datapool_data.json"
              << std::endl;
    if (in.is_open()) {
      std::string line;
      while (std::getline(in, line)) {
        std::cout << line << std::endl;
      }
      in.close();
    }
  }

  // Now we will access the data
  tinc::DataPool dp(ps);
  dp.registerDataFile("datapool_data.json", "internalValuesDim");

  // You can write slices disk (with automatic caching)
  internalValuesDim->setCurrentIndex(0);
  auto dataSliceFile = dp.createDataSlice("value", "internalValuesDim");
  std::cout << "Slice written to " << dataSliceFile << std::endl;

  internalValuesDim->setCurrentIndex(1);
  dataSliceFile = dp.createDataSlice("value", "internalValuesDim");
  std::cout << "Slice written to " << dataSliceFile << std::endl;

  // Or you can request a slice to memory.
  // By default this will write a file or read the file if already produced
  size_t readCount = 0;

  float slice[5];
  internalValuesDim->setCurrentIndex(0);
  dp.readDataSlice("value", "internalValuesDim", slice, 5);
  for (size_t i = 0; i < 5; i++) {
    std::cout << slice[i] << " ";
  }
  std::cout << std::endl;
  internalValuesDim->setCurrentIndex(1);
  dp.readDataSlice("value", "internalValuesDim", slice, 5);
  for (size_t i = 0; i < 5; i++) {
    std::cout << slice[i] << " ";
  }
  std::cout << std::endl;
  internalValuesDim->setCurrentIndex(2);
  dp.readDataSlice("value", "internalValuesDim", slice, 5);
  for (size_t i = 0; i < 5; i++) {
    std::cout << slice[i] << " ";
  }
  std::cout << std::endl;

  return 0;
}
