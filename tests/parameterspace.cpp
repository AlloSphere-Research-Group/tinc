#include "gtest/gtest.h"

#include "tinc/ProcessorCpp.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(ParameterSpace, Basic) {
  ParameterSpace ps;
  ps.newDimension("dim1");
  ps.newDimension("dim2");
  ps.newDimension("dim3");

  auto dimensionNames = ps.getDimensionNames();
  EXPECT_EQ(dimensionNames.size(), 3);
  EXPECT_EQ(dimensionNames.at(0), "dim1");
  EXPECT_EQ(dimensionNames.at(1), "dim2");
  EXPECT_EQ(dimensionNames.at(2), "dim3");

  ps.removeDimension("dim1");

  EXPECT_EQ(ps.getDimensionNames().size(), 2);
  ps.clear();

  EXPECT_EQ(ps.getDimensions().size(), 0);

  EXPECT_EQ(ps.getDimension("no_dim"), nullptr);
}

TEST(ParameterSpace, DimensionValues) {
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  std::vector<float> values = {-0.25, -0.125, 0.0, 0.125, 0.25};
  dim1->setSpaceValues(values);

  EXPECT_EQ(dim1->size(), values.size());
  auto setValues = dim1->getSpaceValues<float>();
  for (size_t i = 0; i < values.size(); i++) {

    EXPECT_EQ(setValues[i], values[i]);
  }

  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::VALUE,
                              ParameterType::PARAMETER_INT32);
  std::vector<int32_t> valuesInt32 = {-6, -3, -2, -1, 0, 5, 6, 7, 8, 10};
  dim3->setSpaceValues(valuesInt32);

  EXPECT_EQ(dim3->size(), valuesInt32.size());
  auto setInt32Values = dim3->getSpaceValues<int32_t>();
  for (size_t i = 0; i < valuesInt32.size(); i++) {

    EXPECT_EQ(setInt32Values[i], valuesInt32[i]);
  }

  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::VALUE,
                              ParameterType::PARAMETER_INT64);
  std::vector<int64_t> valuesInt = {-3, -2, -1, 0, 5, 6, 7, 8};
  dim2->setSpaceValues(valuesInt);

  EXPECT_EQ(dim2->size(), valuesInt.size());
  auto setIntValues = dim2->getSpaceValues<int64_t>();
  for (size_t i = 0; i < valuesInt.size(); i++) {

    EXPECT_EQ(setIntValues[i], valuesInt[i]);
  }

  // TODO verify dimension space setting for all types.
}

TEST(ParameterSpace, DimensionReregister) {
  ParameterSpace ps;

  auto dim1 = ps.newDimension("dim1");
  dim1->setSpaceRepresentationType(ParameterSpaceDimension::VALUE);

  float values[] = {0.1, 0.2, 0.3};
  dim1->setSpaceValues(values, 3, "prefix");

  auto newDim = std::make_shared<ParameterSpaceDimension>("dim1");

  newDim->setSpaceRepresentationType(ParameterSpaceDimension::ID);
  float aliasValues[] = {0.4, 0.5, 0.6, 0.7};
  newDim->setSpaceValues(aliasValues, 4);
  newDim->setSpaceIds({"A", "B", "C", "C", "E"});

  auto newDim1 = ps.registerDimension(newDim);

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
  //                                   ParameterType::PARAMETER_STRING);
  //  auto doubleDim = ps.newDimension("doubleDim",
  //  ParameterSpaceDimension::VALUE,
  //                                   ParameterType::PARAMETER_DOUBLE);
  auto float8Dim = ps.newDimension("floatDim", ParameterSpaceDimension::VALUE,
                                   ParameterType::PARAMETER_FLOAT);
  EXPECT_TRUE(float8Dim != nullptr);
  //  auto int64Dim = ps.newDimension("int64Dim",
  //  ParameterSpaceDimension::VALUE,
  //                                  ParameterType::PARAMETER_INT64);
  auto int32Dim = ps.newDimension("int32Dim", ParameterSpaceDimension::VALUE,
                                  ParameterType::PARAMETER_INT32);
  EXPECT_TRUE(int32Dim != nullptr);
  auto int8Dim = ps.newDimension("int8Dim", ParameterSpaceDimension::VALUE,
                                 ParameterType::PARAMETER_INT8);
  EXPECT_TRUE(int8Dim != nullptr);
  //  auto uint64Dim = ps.newDimension("uint64Dim",
  //  ParameterSpaceDimension::VALUE,
  //                                   ParameterType::PARAMETER_UINT64);
  //  auto uint32Dim = ps.newDimension("uint32Dim",
  //  ParameterSpaceDimension::VALUE,
  //                                   ParameterType::PARAMETER_UINT32);
  auto uint8Dim = ps.newDimension("uint8Dim", ParameterSpaceDimension::VALUE,
                                  ParameterType::PARAMETER_UINT8);
  EXPECT_TRUE(uint8Dim != nullptr);

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

  auto name = ps.resolveTemplate("file_%%dim1%%_%%dim2%%_%%dim3%%");
  EXPECT_EQ(name, "file_0.500000_1_id2");

  name = ps.resolveTemplate("file_%%dim2:VALUE%%_%%dim3:VALUE%%");
  EXPECT_EQ(name, "file_0.200000_0.020000");

  name = ps.resolveTemplate("file_%%dim2:ID%%_%%dim3:ID%%");
  EXPECT_EQ(name, "file_xx0.200000_id2");

  name = ps.resolveTemplate("file_%%dim2:INDEX%%_%%dim3:INDEX%%");
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
  //                              ParameterType::PARAMETER_INT32);

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
    al::File f("out.txt", "w", true);
    std::string text =
        std::to_string(proc.configuration["dim1"].get<float>()) + "_" +
        std::to_string(proc.configuration["dim2"].get<uint64_t>()) + "_" +
        proc.configuration["dim3"].get<std::string>();
    f.write(text);
    return true;
  };

  int counter = 0;
  proc.registerDoneCallback([&](bool success) {
    if (success) {
      counter++;
    }
  });
  ps.setRootPath("ps_test");
  ps.createDataDirectories();
  ps.sweep(proc);

  EXPECT_EQ(counter, 4 * 5 * 6);

  for (auto path : ps.runningPaths()) {
    EXPECT_TRUE(al::File::isDirectory(path));
    EXPECT_TRUE(al::File::exists(path + "out.txt"));
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

TEST(ParameterSpace, CommonId) {
  tinc::ParameterSpace ps{"PS"};
  auto dimension1 =
      ps.newDimension("dim1", tinc::ParameterSpaceDimension::VALUE);
  auto dimension2 =
      ps.newDimension("dim2", tinc::ParameterSpaceDimension::VALUE);

  // Set possible values for dimensions
  dimension1->setSpaceValues<float>({0.1, 0.1, 0.2, 0.2, 0.3, 0.3});
  dimension1->setSpaceIds({"A", "B", "C", "D", "E", "F"});
  dimension1->conformSpace();

  dimension2->setSpaceValues<float>({10.1, 10.2, 10.1, 10.2, 10.1, 10.2});
  dimension2->setSpaceIds({"A", "B", "C", "D", "E", "F"});
  dimension2->conformSpace();

  dimension1->setCurrentIndex(0);
  dimension2->setCurrentIndex(0);

  EXPECT_EQ(dimension1->getCurrentIds(), (std::vector<std::string>{"A", "B"}));
  dimension1->stepIncrement();
  EXPECT_EQ(dimension1->getCurrentId(), "C");
  EXPECT_EQ(dimension1->getCurrentIds(), (std::vector<std::string>{"C", "D"}));

  dimension1->stepIncrement();
  EXPECT_EQ(dimension1->getCurrentId(), "E");
  EXPECT_EQ(dimension1->getCurrentIds(), (std::vector<std::string>{"E", "F"}));

  EXPECT_EQ(dimension2->getCurrentIds(),
            (std::vector<std::string>{"A", "C", "E"}));
  dimension1->stepIncrement();
  EXPECT_EQ(dimension1->getCurrentId(), "E");
  EXPECT_EQ(dimension1->getCurrentIds(), (std::vector<std::string>{"E", "F"}));

  dimension2->stepIncrement();
  EXPECT_EQ(ps.getCommonId(), "F");
  dimension2->stepDecrease();
  EXPECT_EQ(ps.getCommonId(), "E");
  dimension1->stepDecrease();
  dimension2->stepIncrement();
  EXPECT_EQ(ps.getCommonId(), "D");
  dimension2->stepDecrease();
  EXPECT_EQ(ps.getCommonId(), "C");
  dimension1->stepDecrease();
  dimension2->stepIncrement();
  EXPECT_EQ(ps.getCommonId(), "B");
  dimension2->stepDecrease();
  EXPECT_EQ(ps.getCommonId(), "A");
}

TEST(ParameterSpace, MultiIDDimensions) {
  ParameterSpace ps;
  ps.setRootPath(TINC_TESTS_SOURCE_DIR "/data");

  // This internal dimension determines the index into the elements found in
  // results.json
  auto internalDim = ps.newDimension("internal");
  internalDim->setSpaceValues(
      std::vector<float>{0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7});
  internalDim->conformSpace();
  auto externalDim1 = ps.newDimension("external1", ParameterSpaceDimension::ID);
  auto externalDim2 =
      ps.newDimension("external2", ParameterSpaceDimension::VALUE);
  externalDim1->setSpaceValues(std::vector<float>{10.0, 10.0, 10.1, 10.1});
  externalDim1->setSpaceIds(
      {"folderA_1", "folderA_2", "folderB_1", "folderB_2"});
  externalDim1->conformSpace();
  externalDim2->setSpaceValues(std::vector<float>{1, 1, 2, 2});
  externalDim2->setSpaceIds(
      {"folderA_1", "folderB_1", "folderA_2", "folderB_2"});
  externalDim2->conformSpace();

  ps.setCurrentPathTemplate("%%external1,external2%%/");
  EXPECT_TRUE(
      al::File::isSamePath(ps.getCurrentRelativeRunPath(), "folderA_1/"));
  externalDim1->setCurrentValue(10.1);
  EXPECT_TRUE(
      al::File::isSamePath(ps.getCurrentRelativeRunPath(), "folderB_1/"));
  externalDim2->setCurrentValue(2);
  EXPECT_TRUE(
      al::File::isSamePath(ps.getCurrentRelativeRunPath(), "folderB_2/"));
  externalDim1->setCurrentValue(10.0);
  EXPECT_TRUE(
      al::File::isSamePath(ps.getCurrentRelativeRunPath(), "folderA_2/"));
  externalDim2->setCurrentValue(1);
  EXPECT_TRUE(
      al::File::isSamePath(ps.getCurrentRelativeRunPath(), "folderA_1/"));
  std::cout << std::endl;
}

TEST(ParameterSpace, ReadWriteNetCDFSpace) {

  { // int
    ParameterSpace ps;

    auto dim_int = ps.newDimension("dim_int");
    std::vector<int> dim_int_values = {14, 43, 55, 114};
    dim_int->setSpaceValues(dim_int_values.data(), dim_int_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_int = ps.getDimension("dim_int")->getSpaceValues<int>();
    EXPECT_EQ(values_int, dim_int_values);
  }
  { // int8
    ParameterSpace ps;
    auto dim_int8 = ps.newDimension("dim_int8", ParameterSpaceDimension::VALUE,
                                    ParameterType::PARAMETER_INT8);
    std::vector<int8_t> dim_int8_values = {-1, 4, 45, 104};
    dim_int8->setSpaceValues(dim_int8_values.data(), dim_int8_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_int8 = ps.getDimension("dim_int8")->getSpaceValues<int8_t>();
    EXPECT_EQ(values_int8, dim_int8_values);
  }
  { // int16
    ParameterSpace ps;
    auto dim_int16 =
        ps.newDimension("dim_int16", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_INT16);
    std::vector<int16_t> dim_int16_values = {-12, 43, 345, 404};
    dim_int16->setSpaceValues(dim_int16_values.data(), dim_int16_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_int16 = ps.getDimension("dim_int16")->getSpaceValues<int16_t>();
    EXPECT_EQ(values_int16, dim_int16_values);
  }
  { // int32
    ParameterSpace ps;
    auto dim_int32 =
        ps.newDimension("dim_int32", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_INT32);
    std::vector<int32_t> dim_int32_values = {-24, 53, 645, 1404};
    dim_int32->setSpaceValues(dim_int32_values.data(), dim_int32_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_int32 = ps.getDimension("dim_int32")->getSpaceValues<int32_t>();
    EXPECT_EQ(values_int32, dim_int32_values);
  }
  { // int64
    ParameterSpace ps;
    auto dim_int64 =
        ps.newDimension("dim_int64", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_INT64);
    std::vector<int64_t> dim_int64_values = {-155, 253, 324, 1210};
    dim_int64->setSpaceValues(dim_int64_values.data(), dim_int64_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_int64 = ps.getDimension("dim_int64")->getSpaceValues<int64_t>();
    EXPECT_EQ(values_int64, dim_int64_values);
  }
  { // uint8
    ParameterSpace ps;
    auto dim_uint8 =
        ps.newDimension("dim_uint8", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_UINT8);
    std::vector<uint8_t> dim_uint8_values = {1, 4, 45, 104};
    dim_uint8->setSpaceValues(dim_uint8_values.data(), dim_uint8_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_uint8 = ps.getDimension("dim_uint8")->getSpaceValues<uint8_t>();
    EXPECT_EQ(values_uint8, dim_uint8_values);
  }
  { // uint16
    ParameterSpace ps;
    auto dim_uint16 =
        ps.newDimension("dim_uint16", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_UINT16);
    std::vector<uint16_t> dim_uint16_values = {12, 43, 345, 404};
    dim_uint16->setSpaceValues(dim_uint16_values.data(),
                               dim_uint16_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_uint16 =
        ps.getDimension("dim_uint16")->getSpaceValues<uint16_t>();
    EXPECT_EQ(values_uint16, dim_uint16_values);
  }
  { // uint32
    ParameterSpace ps;
    auto dim_uint32 =
        ps.newDimension("dim_uint32", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_UINT32);
    std::vector<uint32_t> dim_uint32_values = {24, 53, 645, 1404};
    dim_uint32->setSpaceValues(dim_uint32_values.data(),
                               dim_uint32_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_uint32 =
        ps.getDimension("dim_uint32")->getSpaceValues<uint32_t>();
    EXPECT_EQ(values_uint32, dim_uint32_values);
  }
  { // uint64
    ParameterSpace ps;
    auto dim_uint64 =
        ps.newDimension("dim_uint64", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_UINT64);
    std::vector<uint64_t> dim_uint64_values = {155, 253, 324, 1210};
    dim_uint64->setSpaceValues(dim_uint64_values.data(),
                               dim_uint64_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_uint64 =
        ps.getDimension("dim_uint64")->getSpaceValues<uint64_t>();
    EXPECT_EQ(values_uint64, dim_uint64_values);
  }
  { // float
    ParameterSpace ps;
    auto dim_float =
        ps.newDimension("dim_float", ParameterSpaceDimension::VALUE,
                        ParameterType::PARAMETER_FLOAT);
    std::vector<float> dim_float_values = {324, 453, 1645, 14210};
    dim_float->setSpaceValues(dim_float_values.data(), dim_float_values.size());
    ps.writeToNetCDF("parameter_space_testing.nc");
    ps.clear();
    ps.readFromNetCDF("parameter_space_testing.nc");
    auto values_float = ps.getDimension("dim_float")->getSpaceValues<float>();
    EXPECT_EQ(values_float, dim_float_values);
  }
  { // String
    // ParameterSpace ps;
    // auto dim_string =
    //     ps.newDimension("dim_string", ParameterSpaceDimension::VALUE,
    //                     ParameterType::PARAMETER_STRING);
    // std::string dim_string_values = "Test String 123";
    // dim_string->setSpaceValues(dim_string_values.data(), dim_string_values.size());
    // ps.writeToNetCDF("parameter_space_testing.nc");
    // ps.clear();
    // ps.readFromNetCDF("parameter_space_testing.nc");
    // auto values_string = ps.getDimension("dim_string")->getSpaceValues<std::string>();
    // EXPECT_EQ(values_string, dim_string_values);
  }
  /*
  // int32
  auto dim_int32 = ps.newDimension("dim_int8",ParameterSpaceDimension::VALUE,
                                   ParameterType::PARAMETER_INT32);
  int8_t dim_int32_values[4] = {14, 143, 155, 214};
  dim_int32->setSpaceValues(dim_int32_values, 4);
  ps.writeToNetCDF("parameter_space_testing.nc");
  ps.clear();
  ps.readFromNetCDF("parameter_space_testing.nc");
  auto values_int32 = ps.getDimension("dim_int32")->getSpaceValues<int32_t>();
  EXPECT_EQ(values_int32, std::vector<int>({14, 143, 155, 21}));
*/

  // Create a parameter space

  // TODO ML Implement

  //    ps.writeToNetCDF();

  //    ps.readFromNetCDF();

  //    // test that the parameter space was written and read correctly for all
  //    possible types

  //    ps.writeToNetCDF("alternate_name.nc");

  //    ps.readFromNetCDF("alternate_name.nc");

  //    // Test that changin name is not an issue
}
