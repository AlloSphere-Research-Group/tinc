#include "tinc/ProcessorCpp.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/io/al_File.hpp"

#include <fstream>

int main() {
  tinc::ParameterSpace ps;

  // Create dimensions
  auto dimension1 = ps.newDimension("dim1", tinc::ParameterSpaceDimension::ID);
  auto dimension2 =
      ps.newDimension("dim2", tinc::ParameterSpaceDimension::INDEX);
  auto inner_param =
      ps.newDimension("inner_param", tinc::ParameterSpaceDimension::VALUE);

  // Set possible values for dimensions
  dimension1->setSpaceValues<float>({0.1, 0.2, 0.3, 0.4, 0.5});
  dimension1->setSpaceIds({"A", "B", "C", "D", "E"});

  dimension2->setSpaceValues<float>({10.1, 10.2, 10.3, 10.4, 10.5});

  inner_param->setSpaceValues<float>({1, 2, 3, 4, 5});

  // Write paramter space information to netcdf file.
  if (!ps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Now load the parameter space from disk
  tinc::ParameterSpace ps2;

  // Because they share the same root directory, the parameter spaces will share
  // the same dimensions
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
