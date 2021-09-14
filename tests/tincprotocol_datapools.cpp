#include "gtest/gtest.h"

#include "tinc/DataPoolJson.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(DataPoolProtocol, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  // TODO ML check that the parameter space details in the data pool are correct

  tclient.stop();
  tserver.stop();
}

TEST(DataPoolProtocol, Metadata) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  // Set up parameter space and data pool
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

  tserver << dp; // Should register everything

  // Connect client

  TincClient tclient;
  //  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());

  auto externalDataPool =
      static_cast<DataPoolJson *>(tclient.getDataPool("dp"));

  auto regFiles = dp.getRegisteredDataFiles();
  auto clientRegFiles = externalDataPool->getRegisteredDataFiles();

  EXPECT_EQ(regFiles.size(), clientRegFiles.size());

  for (auto &regFile : regFiles) {

    EXPECT_EQ(regFile.second, clientRegFiles[regFile.first]);
  }

  tclient.stop();
  tserver.stop();
}

TEST(DataPoolProtocol, Slice) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  // Set up parameter space and data pool
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

  ps.setCurrentPathTemplate("%%external:ID%%/");

  DataPoolJson dp{"dp", ps, "sliceCache"};
  dp.registerDataFile("results.json", "internal");

  tserver << dp; // Should register dp dependencies

  // Connect client

  TincClient tclient;
  //  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());

  // Now set values from client and slice data pool
  auto internalDimClient = tclient.getDimension("internal");
  auto externalDimClient = tclient.getDimension("external");

  auto dataPoolClient = static_cast<DataPoolJson *>(tclient.getDataPool("dp"));

  internalDimClient->setCurrentValue(0.0);
  externalDimClient->setCurrentValue(10.0);
  double data[10];
  auto sliceSize =
      dataPoolClient->readDataSlice("field1", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 1);
  EXPECT_EQ(data[2], 5);

  sliceSize = dataPoolClient->readDataSlice("field2", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 0);
  EXPECT_EQ(data[2], 6);

  sliceSize = dataPoolClient->readDataSlice("field3", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 1);

  internalDimClient->setCurrentValue(0.4);

  sliceSize = dataPoolClient->readDataSlice("field1", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 4);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 0);

  sliceSize = dataPoolClient->readDataSlice("field2", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 5);
  EXPECT_EQ(data[1], 4);
  EXPECT_EQ(data[2], 1);

  sliceSize = dataPoolClient->readDataSlice("field3", "external", data, 10);
  EXPECT_EQ(sliceSize, 3);
  EXPECT_EQ(data[0], 8);
  EXPECT_EQ(data[1], 0);
  EXPECT_EQ(data[2], 9);

  // Now test internal, where slicing occurs within a single file
  sliceSize = dataPoolClient->readDataSlice("field1", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 0);
  EXPECT_EQ(data[1], 1);
  EXPECT_EQ(data[2], 2);
  EXPECT_EQ(data[3], 3);
  EXPECT_EQ(data[4], 4);
  EXPECT_EQ(data[5], 5);
  EXPECT_EQ(data[6], 6);
  EXPECT_EQ(data[7], 7);

  sliceSize = dataPoolClient->readDataSlice("field2", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 2);
  EXPECT_EQ(data[2], 3);
  EXPECT_EQ(data[3], 4);
  EXPECT_EQ(data[4], 5);
  EXPECT_EQ(data[5], 6);
  EXPECT_EQ(data[6], 7);
  EXPECT_EQ(data[7], 8);

  sliceSize = dataPoolClient->readDataSlice("field3", "internal", data, 10);
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
  externalDimClient->setCurrentValue(10.2);

  sliceSize = dataPoolClient->readDataSlice("field1", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 5);
  EXPECT_EQ(data[1], 7);
  EXPECT_EQ(data[2], 8);
  EXPECT_EQ(data[3], 9);
  EXPECT_EQ(data[4], 0);
  EXPECT_EQ(data[5], 1);
  EXPECT_EQ(data[6], 6);
  EXPECT_EQ(data[7], 4);

  sliceSize = dataPoolClient->readDataSlice("field2", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 6);
  EXPECT_EQ(data[1], 7);
  EXPECT_EQ(data[2], 8);
  EXPECT_EQ(data[3], 0);
  EXPECT_EQ(data[4], 1);
  EXPECT_EQ(data[5], 3);
  EXPECT_EQ(data[6], 5);
  EXPECT_EQ(data[7], 7);

  sliceSize = dataPoolClient->readDataSlice("field3", "internal", data, 10);
  EXPECT_EQ(sliceSize, 8);
  EXPECT_EQ(data[0], 1);
  EXPECT_EQ(data[1], 3);
  EXPECT_EQ(data[2], 5);
  EXPECT_EQ(data[3], 7);
  EXPECT_EQ(data[4], 9);
  EXPECT_EQ(data[5], 0);
  EXPECT_EQ(data[6], 2);
  EXPECT_EQ(data[7], 4);

  tclient.stop();
  tserver.stop();
}
