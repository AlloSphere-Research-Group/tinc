#include "gtest/gtest.h"

#include "tinc/ParameterSpace.hpp"
#include "tinc/ParameterSpaceDimension.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/CacheManager.hpp"
#include "al/io/al_File.hpp"

#include <ctime>
#include <chrono>
#include <fstream>

using namespace tinc;

TEST(Cache, Basic) {
  if (al::File::exists("basic_cache.json")) {
    al::File::remove("basic_cache.json");
  }
  CacheManager cmanage(DistributedPath{"basic_cache.json"});

  cmanage.writeToDisk();
  cmanage.updateFromDisk();
}

TEST(Cache, ReadWriteEntry) {
  if (al::File::exists("read_write_cache.json")) {
    al::File::remove("read_write_cache.json");
  }
  CacheManager cmanage(DistributedPath{"read_write_cache.json"});

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  time_t start = std::chrono::system_clock::to_time_t(now);
  char startTimestamp[sizeof "2011-10-08T07:07:09Z"];
  strftime(startTimestamp, sizeof startTimestamp, "%FT%TZ",
           std::gmtime(&start));
  time_t end =
      std::chrono::system_clock::to_time_t(now + std::chrono::seconds(10));
  char endTimestamp[sizeof "2011-10-08T07:07:09Z"];
  strftime(endTimestamp, sizeof endTimestamp, "%FT%TZ", std::gmtime(&end));

  CacheEntry entry;
  entry.timestampStart = startTimestamp;
  entry.timestampEnd = endTimestamp;
  entry.cacheHits = 23;
  entry.filenames = {"hello", "world"};
  entry.stale = true;

  entry.userInfo.userName = "MyName";
  entry.userInfo.userHash = "MyHash";
  entry.userInfo.ip = "localhost";
  entry.userInfo.port = 12345;
  entry.userInfo.server = true;

  SourceInfo sourceInfo;
  sourceInfo.type = "SourceType";
  sourceInfo.hash = "SourceHash";
  sourceInfo.commandLineArguments = "args and args";

  SourceArgument arg_int;
  arg_int.id = "int";
  arg_int.value = 450;
  sourceInfo.arguments.push_back(arg_int);

  SourceArgument arg_float;
  arg_float.id = "float";
  arg_float.value = 4.05;
  sourceInfo.arguments.push_back(arg_float);

  SourceArgument arg_string;
  arg_string.id = "string";
  arg_string.value = "hello";
  sourceInfo.arguments.push_back(arg_string);

  // Add dependencies
  sourceInfo.dependencies.push_back(arg_int);
  sourceInfo.dependencies.push_back(arg_float);
  sourceInfo.dependencies.push_back(arg_string);

  entry.sourceInfo = sourceInfo;

  cmanage.appendEntry(entry);
  cmanage.writeToDisk();

  cmanage.updateFromDisk();
  auto entries = cmanage.entries();

  EXPECT_EQ(entries.size(), 1);
  EXPECT_EQ(entries[0].timestampStart, startTimestamp);
  EXPECT_EQ(entries[0].timestampEnd, endTimestamp);
  EXPECT_EQ(entries[0].cacheHits, 23);
  EXPECT_EQ(entries[0].filenames.size(), 2);
  EXPECT_EQ(entries[0].filenames[0], "hello");
  EXPECT_EQ(entries[0].filenames[1], "world");
  EXPECT_TRUE(entries[0].stale);

  EXPECT_EQ(entries[0].userInfo.ip, "localhost");
  EXPECT_EQ(entries[0].userInfo.server, true);
  EXPECT_EQ(entries[0].userInfo.userName, "MyName");
  EXPECT_EQ(entries[0].userInfo.userHash, "MyHash");
  EXPECT_EQ(entries[0].userInfo.ip, "localhost");
  EXPECT_EQ(entries[0].userInfo.port, 12345);

  EXPECT_EQ(entries[0].sourceInfo.type, "SourceType");
  EXPECT_EQ(entries[0].sourceInfo.hash, "SourceHash");
  EXPECT_EQ(entries[0].sourceInfo.commandLineArguments, "args and args");

  EXPECT_EQ(entries[0].sourceInfo.arguments.size(), 3);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(0).id, "int");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(0).value.type, VARIANT_INT64);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(0).value.valueInt64, 450);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(1).id, "float");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(1).value.type, VARIANT_DOUBLE);
  EXPECT_FLOAT_EQ(entries[0].sourceInfo.arguments.at(1).value.valueDouble,
                  4.05);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(2).id, "string");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(2).value.type, VARIANT_STRING);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(2).value.valueStr, "hello");

  EXPECT_EQ(entries[0].sourceInfo.dependencies.size(), 3);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(0).id, "int");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(0).value.type, VARIANT_INT64);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(0).value.valueInt64, 450);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(1).id, "float");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(1).value.type,
            VARIANT_DOUBLE);
  EXPECT_FLOAT_EQ(entries[0].sourceInfo.dependencies.at(1).value.valueDouble,
                  4.05);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(2).id, "string");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(2).value.type,
            VARIANT_STRING);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(2).value.valueStr, "hello");
}

TEST(Cache, ParameterSpace) {
  if (al::File::exists("cache_parameterspace.json")) {
    al::File::remove("cache_parameterspace.json");
  }

  // Make a ParameterSpace
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

  ps.enableCache("cache_parameterspace.json");

  // Create a processor;
  ProcessorCpp processor;
  processor.setOutputFileNames({"cache_ps.txt"});
  processor.processingFunction = [&]() {
    // Compute a value
    float value = processor.configuration["dim1"].valueDouble *
                      processor.configuration["dim2"].valueDouble +
                  processor.configuration["dim3"].valueDouble;

    auto filenames = processor.getOutputFileNames();

    al::al_sleep(3); // Sleep for 3 seconds.
    // Write it to file
    std::ofstream f(filenames[0]);
    f << std::to_string(value);
    f.close();
    return true;
  };

  // Run the processor through the parameter space
  ps.runProcess(processor);

  // Try different values
  dim1->setCurrentValue(0.4);
  dim2->setCurrentValue(0.1);
  dim3->setCurrentValue(0.03);

  ps.runProcess(processor);

  // Try values already used to use cache.
  dim1->setCurrentValue(0.5);
  dim2->setCurrentValue(0.2);
  dim3->setCurrentValue(0.02);

  ps.runProcess(processor);
}
