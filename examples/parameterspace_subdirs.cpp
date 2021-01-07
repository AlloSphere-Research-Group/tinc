#include "tinc/ProcessorCpp.hpp"
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

  ps.rootPath = "subdir_data/";
  ps.createDataDirectories();

  if (!ps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Make a change for one  of the data directories in a dimension
  tinc::ParameterSpace subdirps;

  auto dimension2b = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
  dimension2b->setSpaceValues({20.1, 20.2, 20.3, 20.4, 20.5});
  dimension2b->setSpaceRepresentationType(tinc::ParameterSpaceDimension::INDEX);

  subdirps.registerDimension(dimension2b);

  subdirps.rootPath = "subdir_data/B/";
  subdirps.createDataDirectories();

  if (!subdirps.writeToNetCDF()) {
    std::cerr << "Error writing NetCDF file" << std::endl;
  }

  // Now load to confirm the variation in the subdirectory is picked up.
  tinc::ParameterSpace ps2;
  ps2.rootPath = "subdir_data/";
  ps2.readFromNetCDF();

  ps2.getDimension("dim1")->setCurrentIndex(0);

  auto value1 = ps.getDimension("dim2")->at(0);
  auto value2 = ps2.getDimension("dim2")->at(0);

  std::cout << value1 << " -- " << value2 << std::endl;

  // This should trigger read of parameter space
  ps2.getDimension("dim1")->setCurrentIndex(1);
  value2 = ps2.getDimension("dim2")->at(0);
  std::cout << value1 << " -- " << value2 << std::endl;

  // Should revert to root parameter space
  ps2.getDimension("dim1")->setCurrentIndex(0);
  value2 = ps2.getDimension("dim2")->at(0);
  std::cout << value1 << " -- " << value2 << std::endl;

  return 0;
}
