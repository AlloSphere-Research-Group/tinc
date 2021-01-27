#include "tinc/ProcessorCpp.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/io/al_File.hpp"

#include <fstream>

int main() {
  auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
  auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
  auto inner_param =
      std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

  dimension1->setSpaceValues<float>({0.1, 0.2, 0.3, 0.4, 0.5});
  dimension1->setSpaceIds({"A", "B", "C", "D", "E"});
  dimension1->setSpaceRepresentationType(tinc::ParameterSpaceDimension::ID);

  dimension2->setSpaceValues<float>({10.1, 10.2, 10.3, 10.4, 10.5});
  dimension2->setSpaceRepresentationType(tinc::ParameterSpaceDimension::INDEX);

  inner_param->setSpaceValues<float>({1, 2, 3, 4, 5});
  inner_param->setSpaceRepresentationType(tinc::ParameterSpaceDimension::VALUE);

  tinc::ParameterSpace ps;

  ps.registerDimension(dimension1);
  ps.registerDimension(dimension2);
  ps.registerDimension(inner_param);

  tinc::ProcessorCpp processor;

  processor.setOutputFileNames({"output.txt"});

  processor.processingFunction = [&]() {
    std::string text =
        processor.configuration["dim1"].valueStr + " -- " +
        std::to_string(processor.configuration["dim2"].valueInt64) + " -- " +
        std::to_string(processor.configuration["inner_param"].valueDouble);

    std::cout << "Writing value: "
              << processor.configuration["inner_param"].valueDouble << " to: "
              << processor.getRunningDirectory() +
                     processor.getOutputFileNames()[0]
              << std::endl;
    std::ofstream f(processor.getRunningDirectory() +
                        processor.getOutputFileNames()[0],
                    std::ofstream::app);
    f << text << std::endl;
    f.close();

    return true;
  };

  ps.setRootPath("data/");

  // Now sweep the parameter space
  ps.sweep(processor);

  return 0;
}
