#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(ParameterSpace, Basic) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2");
  auto dim3 = ps.newDimension("dim3");

  auto dimensionNames = ps.dimensionNames();
  EXPECT_EQ(dimensionNames.size(), 3);
  EXPECT_EQ(dimensionNames.at(0), "dim1");
  EXPECT_EQ(dimensionNames.at(1), "dim2");
  EXPECT_EQ(dimensionNames.at(2), "dim3");
  ps.clear();

  EXPECT_EQ(ps.getDimensions().size(), 0);
}

TEST(ParameterSpace, DimensionValues) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  std::vector<float> values = {-0.25, -0.125, 0.0, 0.125, 0.25};
  dim1->setSpaceValues(values);

  EXPECT_EQ(dim1->size(), 5);
  auto setValues = dim1->getSpaceValues<float>();
  for (size_t i = 0; i < 5; i++) {

    EXPECT_EQ(setValues[i], values[i]);
  }

  // TODO verify dimension space setting for all types.
}

TEST(ParameterSpace, DimensionReregister) {
  ParameterSpace ps;

  auto dim1 = ps.newDimension("dim1");
  dim1->setSpaceRepresentationType(ParameterSpaceDimension::VALUE);

  float values[] = {0.1, 0.2, 0.3};
  dim1->setSpaceValues(values, 3, "prefix");

  auto newDim1 = std::make_shared<ParameterSpaceDimension>("dim1");

  newDim1->setSpaceRepresentationType(ParameterSpaceDimension::ID);
  float aliasValues[] = {0.4, 0.5, 0.6, 0.7};
  newDim1->setSpaceValues(aliasValues, 4);
  newDim1->setSpaceIds({"A", "B", "C", "C", "E"});

  newDim1 = ps.registerDimension(newDim1);

  // The dimension pointer should correspond to the previously registered one
  EXPECT_EQ(newDim1, dim1);
  // But all the properties of the dimension should have been copied.
  EXPECT_EQ(newDim1->getSpaceRepresentationType(), ParameterSpaceDimension::ID);
  EXPECT_EQ(newDim1->size(), 4);
  EXPECT_EQ(newDim1->getSpaceIds(),
            std::vector<std::string>({"A", "B", "C", "C", "E"}));
}

TEST(ParameterSpace, DimensionAlias) {
  ParameterSpace ps;

  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2");

  ps.parameterNameMap = {{"dim1Alias", "dim1"}, {"dim2Alias", "dim2"}};

  EXPECT_EQ(dim1, ps.getDimension("dim1"));
  EXPECT_EQ(dim2, ps.getDimension("dim2"));
  EXPECT_EQ(dim1, ps.getDimension("dim1Alias"));
  EXPECT_EQ(dim2, ps.getDimension("dim2Alias"));
}

TEST(ParameterSpace, FilenameTemplate) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);

  float values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->setSpaceValues(values, 5, "xx");

  float dim3Values[6];
  std::vector<std::string> ids;
  for (int i = 0; i < 6; i++) {
    dim3Values[i] = i * 0.01;
    ids.push_back("id" + std::to_string(i));
  }
  dim3->setSpaceValues(dim3Values, 6);
  dim3->setSpaceIds(ids);

  dim1->setCurrentValue(0.5);
  dim2->setCurrentValue(0.2);
  dim3->setCurrentValue(0.02);

  auto name = ps.resolveFilename("file_%%dim1%%_%%dim2%%_%%dim3%%");
  EXPECT_EQ(name, "file_0.500000_1_id2");

  name = ps.resolveFilename("file_%%dim2:VALUE%%_%%dim3:VALUE%%");
  EXPECT_EQ(name, "file_0.200000_0.020000");

  name = ps.resolveFilename("file_%%dim2:ID%%_%%dim3:ID%%");
  EXPECT_EQ(name, "file_xx0.200000_id2");
}

TEST(ParameterSpace, RunningPaths) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);

  float dim1Values[5] = {0.1, 0.2, 0.3, 0.4};
  dim1->setSpaceValues(dim1Values, 4);

  float dim2Values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->setSpaceValues(dim2Values, 5, "xx");

  float dim3Values[6];
  std::vector<std::string> ids;
  for (int i = 0; i < 6; i++) {
    dim3Values[i] = i * 0.01;
    ids.push_back("id" + std::to_string(i));
  }
  dim3->setSpaceValues(dim3Values, 6);
  dim3->setSpaceIds(ids);

  // Use only dimensions 1 and 2 in path template
  ps.setCurrentPathTemplate("file_%%dim1%%_%%dim2%%");
  EXPECT_EQ(ps.runningPaths().size(), 20);
}

TEST(ParameterSpace, ReadWriteNetCDF) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);

  float dim1Values[5] = {0.1, 0.2, 0.3, 0.4};
  dim1->setSpaceValues(dim1Values, 4);

  float dim2Values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->setSpaceValues(dim2Values, 5);

  float dim3Values[6] = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
  dim3->setSpaceValues(dim3Values, 6);

  ps.writeToNetCDF("parameter_space_testing.nc");
  ps.clear();

  EXPECT_EQ(ps.getDimensions().size(), 0);

  ps.readFromNetCDF("parameter_space_testing.nc");

  EXPECT_EQ(ps.getDimensions().size(), 3);
}
