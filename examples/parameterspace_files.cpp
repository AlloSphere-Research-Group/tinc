#include "tinc/CppProcessor.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/io/al_File.hpp"

#include <fstream>

int main() {
  auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
  auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
  auto inner_param =
      std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

  dimension1->setSpaceValues({0.1, 0.2, 0.3, 0.4, 0.5});
  dimension1->setSpaceIds({"A", "B", "C", "D", "E"});
  dimension1->setSpaceRepresentationType(tinc::ParameterSpaceDimension::ID);

  dimension2->setSpaceValues({10.1, 10.2, 10.3, 10.4, 10.5});
  dimension2->setSpaceRepresentationType(tinc::ParameterSpaceDimension::INDEX);

  inner_param->setSpaceValues({1, 2, 3, 4, 5});
  inner_param->setSpaceRepresentationType(tinc::ParameterSpaceDimension::VALUE);

  tinc::ParameterSpace ps;

  ps.registerDimension(dimension1);
  ps.registerDimension(dimension2);
  ps.registerDimension(inner_param);

  if (!ps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Now load the parameter space from disk
  tinc::ParameterSpace ps2;

  ps2.readFromNetCDF();

  for (auto dimensionName : ps2.dimensionNames()) {
    std::cout << " ---- Dimension: " << dimensionName << std::endl;
    auto dim = ps2.getDimension(dimensionName);

    std::cout << "  Values: " << std::endl;
    for (auto value : dim->getSpaceValues<float>()) {
      std::cout << value << std::endl;
    }
    if (dim->getSpaceIds().size() > 0) {
      std::cout << "  Ids: " << std::endl;
      for (auto id : dim->getSpaceIds()) {
        std::cout << id << std::endl;
      }
    }
  }

  return 0;
}
