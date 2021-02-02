#include "tinc/ParameterSpace.hpp"

int main() {
  tinc::ParameterSpace ps;
  ps.readFromNetCDF(
      "/Users/lt01/code/casm_viewer/vdv_data/NaCoO2_diffusion/monte_with_com/"
      "cached_output/_parameter_space.nc");

  for (auto dimension : ps.getDimensions()) {
    if (dimension->getSpaceRepresentationType() ==
        tinc::ParameterSpaceDimension::VALUE) {
      std::cout << " ***** Internal Parameter: " << dimension->getName()
                << " size: " << dimension->size() << std::endl;
      for (auto value : dimension->getSpaceValues<float>()) {
        std::cout << value << " ";
      }
      std::cout << std::endl;
    } else if (dimension->getSpaceRepresentationType() ==
               tinc::ParameterSpaceDimension::INDEX) {
      std::cout << " ***** Index Parameter: " << dimension->getName()
                << " size: " << dimension->size() << std::endl;
      for (auto value : dimension->getSpaceValues<float>()) {
        std::cout << value << " ";
      }
      std::cout << std::endl;
    } else if (dimension->getSpaceRepresentationType() ==
               tinc::ParameterSpaceDimension::ID) {
      std::cout << " ***** Mapped Parameter: " << dimension->getName()
                << " size: " << dimension->size() << std::endl;
      for (auto value : dimension->getSpaceValues<float>()) {
        std::cout << value << " ";
      }
      std::cout << std::endl;
      // Mapped parameters have ids
      std::cout << " -- ids:" << std::endl;
      for (auto id : dimension->getSpaceIds()) {
        std::cout << "'" << id << "' ";
      }
      std::cout << std::endl;
    }
  }
  return 0;
}
