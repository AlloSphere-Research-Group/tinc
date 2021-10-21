#include "gtest/gtest.h"

#include "tinc/DataPoolJson.hpp"

using namespace tinc;

TEST(Datapool, Basic) {
  ParameterSpace ps{"ps"};

  ps.setRootPath(TINC_TESTS_SOURCE_DIR "/data");

  // This internal dimension determines the index into the elements found in
  // results.json
  auto internalDim = ps.newDimension("internal");
  internalDim->setSpaceValues(
      std::vector<float>{0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7});
  auto externalDim = ps.newDimension("external", ParameterSpaceDimension::ID);
  externalDim->setSpaceValues(std::vector<float>{10.0, 10.1, 10.2});
  externalDim->setSpaceIds({"folder1", "folder2", "folder3"});

  ps.setCurrentPathTemplate("%%external%%/");

  DataPoolJson dp{"dp", ps, "sliceCache"};
  dp.registerDataFile("results.json", "internal");

  auto fields = dp.listFields();
  EXPECT_EQ(fields.size(), 3);
  EXPECT_EQ(fields[0], "field1");
  EXPECT_EQ(fields[1], "field2");
  EXPECT_EQ(fields[2], "field3");
}

TEST(Datapool, Slices) {
  ParameterSpace ps{"ps"};

  ps.setRootPath(TINC_TESTS_SOURCE_DIR "/data");

  // This internal dimension determines the index into the elements found in
  // results.json
  auto internalDim = ps.newDimension("internal");
  internalDim->setSpaceValues(
      std::vector<float>{0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7});
  auto externalDim = ps.newDimension("external", ParameterSpaceDimension::ID);
  externalDim->setSpaceValues(std::vector<float>{10.0, 10.1, 10.2});
  externalDim->setSpaceIds({"folder1", "folder2", "folder3"});

  ps.setCurrentPathTemplate("%%external%%/");

  DataPoolJson dp{"dp", ps, "sliceCache"};
  dp.registerDataFile("results.json", "internal");

  // Slice across files (external dimension)
  internalDim->setCurrentValue(0.0);
  externalDim->setCurrentValue(10.0);
  double data[10];
  auto sliceSize = dp.readDataSlice("field1", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 1);
  EXPECT_EQ(data[2], 5);

  sliceSize = dp.readDataSlice("field2", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 0);
  EXPECT_EQ(data[2], 6);

  sliceSize = dp.readDataSlice("field3", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 1);

  internalDim->setCurrentValue(0.4);

  sliceSize = dp.readDataSlice("field1", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 0);

  sliceSize = dp.readDataSlice("field2", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 5);
  EXPECT_EQ(data[1], 4);
  EXPECT_EQ(data[2], 1);

  sliceSize = dp.readDataSlice("field3", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 8);
  EXPECT_EQ(data[1], 0);
  EXPECT_EQ(data[2], 9);

  // Now test internal, where slicing occurs within a single file
  sliceSize = dp.readDataSlice("field1", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 1);
  EXPECT_EQ(data[2], 2);
  EXPECT_EQ(data[3], 3);
  EXPECT_EQ(data[4], 4);
  EXPECT_EQ(data[5], 5);
  EXPECT_EQ(data[6], 6);
  EXPECT_EQ(data[7], 7);

  sliceSize = dp.readDataSlice("field2", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 2);
  EXPECT_EQ(data[2], 3);
  EXPECT_EQ(data[3], 4);
  EXPECT_EQ(data[4], 5);
  EXPECT_EQ(data[5], 6);
  EXPECT_EQ(data[6], 7);
  EXPECT_EQ(data[7], 8);

  sliceSize = dp.readDataSlice("field3", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 6);
  EXPECT_EQ(data[3], 7);
  EXPECT_EQ(data[4], 8);
  EXPECT_EQ(data[5], 9);
  EXPECT_EQ(data[6], 0);
  EXPECT_EQ(data[7], 1);

  // This sets folder to be 'folder3'
  externalDim->setCurrentValue(10.2);

  sliceSize = dp.readDataSlice("field1", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 5);
  EXPECT_EQ(data[1], 7);
  EXPECT_EQ(data[2], 8);
  EXPECT_EQ(data[3], 9);
  EXPECT_EQ(data[4], 0);
  EXPECT_EQ(data[5], 1);
  EXPECT_EQ(data[6], 6);
  EXPECT_EQ(data[7], 4);

  sliceSize = dp.readDataSlice("field2", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 6);
  EXPECT_EQ(data[1], 7);
  EXPECT_EQ(data[2], 8);
  EXPECT_EQ(data[3], 0);
  EXPECT_EQ(data[4], 1);
  EXPECT_EQ(data[5], 3);
  EXPECT_EQ(data[6], 5);
  EXPECT_EQ(data[7], 7);

  sliceSize = dp.readDataSlice("field3", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 3);
  EXPECT_EQ(data[2], 5);
  EXPECT_EQ(data[3], 7);
  EXPECT_EQ(data[4], 9);
  EXPECT_EQ(data[5], 0);
  EXPECT_EQ(data[6], 2);
  EXPECT_EQ(data[7], 4);
}

TEST(Datapool, SlicesMultiId) {
  ParameterSpace ps{"ps"};

  ps.setRootPath(TINC_TESTS_SOURCE_DIR "/data2");

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

  DataPoolJson dp{"dp", ps, "sliceCache"};
  dp.registerDataFile("results.json", "internal");

  internalDim->setCurrentValue(0.0);
  externalDim1->setCurrentValue(10.0);
  externalDim2->setCurrentValue(1);
  double data[10];
  auto sliceSize = dp.readDataSlice("field1", "external1", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 8);

  sliceSize = dp.readDataSlice("field1", "external2", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 5);

  sliceSize = dp.readDataSlice("field2", "external1", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 5);

  sliceSize = dp.readDataSlice("field2", "external2", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 6);

  sliceSize = dp.readDataSlice("field3", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 6);
  EXPECT_EQ(data[3], 7);
  EXPECT_EQ(data[4], 8);
  EXPECT_EQ(data[5], 9);
  EXPECT_EQ(data[6], 0);
  EXPECT_EQ(data[7], 1);

  //
  internalDim->setCurrentValue(0.1);

  sliceSize = dp.readDataSlice("field1", "external1", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 7);

  sliceSize = dp.readDataSlice("field1", "external2", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 7);

  sliceSize = dp.readDataSlice("field2", "external1", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 2);
  EXPECT_EQ(data[1], 6);

  sliceSize = dp.readDataSlice("field2", "external2", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 2);
  EXPECT_EQ(data[1], 7);

  sliceSize = dp.readDataSlice("field3", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 6);
  EXPECT_EQ(data[3], 7);
  EXPECT_EQ(data[4], 8);
  EXPECT_EQ(data[5], 9);
  EXPECT_EQ(data[6], 0);
  EXPECT_EQ(data[7], 1);

  //
  externalDim1->setCurrentValue(10.1);
  sliceSize = dp.readDataSlice("field1", "external1", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 7);

  sliceSize = dp.readDataSlice("field1", "external2", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 7);
  EXPECT_EQ(data[1], 2);

  sliceSize = dp.readDataSlice("field2", "external1", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 2);
  EXPECT_EQ(data[1], 6);

  sliceSize = dp.readDataSlice("field2", "external2", data, 10);
  EXPECT_EQ(sliceSize, 2);
  EXPECT_EQ(data[0], 6);
  EXPECT_EQ(data[1], 7);

  sliceSize = dp.readDataSlice("field3", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 5);
  EXPECT_EQ(data[1], 7);
  EXPECT_EQ(data[2], 8);
  EXPECT_EQ(data[3], 9);
  EXPECT_EQ(data[4], 0);
  EXPECT_EQ(data[5], 1);
  EXPECT_EQ(data[6], 6);
  EXPECT_EQ(data[7], 4);
}
