#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/ProcessorCpp.hpp"

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

  ps.removeDimension("dim1");

  EXPECT_EQ(ps.dimensionNames().size(), 2);
  ps.clear();

  EXPECT_EQ(ps.getDimensions().size(), 0);

  EXPECT_EQ(ps.getDimension("no_dim"), nullptr);
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

TEST(ParameterSpace, DimensionTypes) {
  ParameterSpace ps;

  //  auto stringDim = ps.newDimension("stringDim",
  //  ParameterSpaceDimension::VALUE,
  //                                   al::DiscreteParameterValues::STRING);
  //  auto doubleDim = ps.newDimension("doubleDim",
  //  ParameterSpaceDimension::VALUE,
  //                                   al::DiscreteParameterValues::DOUBLE);
  auto float8Dim = ps.newDimension("floatDim", ParameterSpaceDimension::VALUE,
                                   al::DiscreteParameterValues::FLOAT);
  //  auto int64Dim = ps.newDimension("int64Dim",
  //  ParameterSpaceDimension::VALUE,
  //                                  al::DiscreteParameterValues::INT64);
  auto int32Dim = ps.newDimension("int32Dim", ParameterSpaceDimension::VALUE,
                                  al::DiscreteParameterValues::INT32);
  auto int8Dim = ps.newDimension("int8Dim", ParameterSpaceDimension::VALUE,
                                 al::DiscreteParameterValues::INT8);
  //  auto uint64Dim = ps.newDimension("uint64Dim",
  //  ParameterSpaceDimension::VALUE,
  //                                   al::DiscreteParameterValues::UINT64);
  //  auto uint32Dim = ps.newDimension("uint32Dim",
  //  ParameterSpaceDimension::VALUE,
  //                                   al::DiscreteParameterValues::UINT32);
  auto uint8Dim = ps.newDimension("uint8Dim", ParameterSpaceDimension::VALUE,
                                  al::DiscreteParameterValues::UINT8);

  EXPECT_EQ(ps.getDimensions().size(), 4);
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

  name = ps.resolveFilename("file_%%dim2:INDEX%%_%%dim3:INDEX%%");
  EXPECT_EQ(name, "file_1_2");
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

  // TODO fix support for int32
  //  auto dim4 = ps.newDimension("dim4", ParameterSpaceDimension::ID,
  //                              al::DiscreteParameterValues::INT32);

  float dim1Values[5] = {0.1, 0.2, 0.3, 0.4};
  dim1->setSpaceValues(dim1Values, 4);

  float dim2Values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->setSpaceValues(dim2Values, 5);

  float dim3Values[6] = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6};
  dim3->setSpaceValues(dim3Values, 6);

  //  int32_t dim4Values[] = {10, 20, 30, 40, 50, 60, 70, 80};
  //  dim4->setSpaceValues(dim4Values, sizeof dim4Values);

  ps.writeToNetCDF("parameter_space_testing.nc");
  ps.clear();

  EXPECT_EQ(ps.getDimensions().size(), 0);

  // Load from netcdf file
  ps.readFromNetCDF("parameter_space_testing.nc");

  EXPECT_EQ(ps.getDimensions().size(), 3);

  // Check values match
  auto values = ps.getDimension("dim1")->getSpaceValues<float>();
  EXPECT_EQ(values, std::vector<float>({0.1, 0.2, 0.3, 0.4}));

  values = ps.getDimension("dim2")->getSpaceValues<float>();
  EXPECT_EQ(values, std::vector<float>({0.1, 0.2, 0.3, 0.4, 0.5}));

  values = ps.getDimension("dim3")->getSpaceValues<float>();
  EXPECT_EQ(values, std::vector<float>({1.1, 1.2, 1.3, 1.4, 1.5, 1.6}));

  //  auto intValues = ps.getDimension("dim4")->getSpaceValues<int32_t>();
  //  EXPECT_EQ(values, std::vector<float>({10, 20, 30, 40, 50, 60, 70, 80}));
}

TEST(ParameterSpace, Sweep) {
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

  ps.setCurrentPathTemplate("file_%%dim1%%_%%dim2%%");

  ProcessorCpp proc("proc");

  proc.processingFunction = [&]() {
    al::File f("out_" + proc.configuration["dim3"].valueStr + ".txt", "w",
               true);
    std::string text = std::to_string(proc.configuration["dim1"].valueDouble) +
                       "_" +
                       std::to_string(proc.configuration["dim2"].valueDouble) +
                       "_" + proc.configuration["dim3"].valueStr;
    f.write(text);
    return true;
  };
  ps.setRootPath("ps_test");
  ps.createDataDirectories();
  ps.sweep(proc);

  for (auto path : ps.runningPaths()) {
    //    EXPECT_FALSE(al::File::isDirectory(path));
  }
}

TEST(ParameterSpace, DataDirectories) {
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

  ps.setCurrentPathTemplate("file_%%dim1%%_%%dim2%%");

  EXPECT_TRUE(ps.cleanDataDirectories());
  for (auto path : ps.runningPaths()) {
    al::Dir::removeRecursively(path); // delete in case it's not a fresh run
    EXPECT_FALSE(al::File::isDirectory(path));
  }

  EXPECT_TRUE(ps.createDataDirectories());
  for (auto path : ps.runningPaths()) {
    EXPECT_TRUE(al::File::isDirectory(path));
  }
  // Generate a file within each directory through a sweep

  ProcessorCpp proc("proc");
  proc.processingFunction = [&]() {
    std::ofstream f("out.txt");
    f << "a";
    f.close();
    return true;
  };

  ps.sweep(proc);

  for (auto path : ps.runningPaths()) {
    EXPECT_EQ(al::itemListInDir(path).count(), 1);
  }

  EXPECT_TRUE(ps.cleanDataDirectories());
  for (auto path : ps.runningPaths()) {
    EXPECT_TRUE(al::File::isDirectory(path));
    EXPECT_EQ(al::itemListInDir(path).count(), 0);
  }

  EXPECT_TRUE(ps.removeDataDirectories());

  for (auto path : ps.runningPaths()) {
    EXPECT_TRUE(!al::File::isDirectory(path));
  }
}
