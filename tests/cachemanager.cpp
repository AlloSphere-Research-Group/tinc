#include "gtest/gtest.h"

#include "al/io/al_File.hpp"
#include "tinc/CacheManager.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/ParameterSpaceDimension.hpp"
#include "tinc/ProcessorCpp.hpp"

#include <chrono>
#include <ctime>
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
  entry.files = {std::string("hello"), std::string("world")};
  entry.stale = true;

  entry.userInfo.userName = "MyName";
  entry.userInfo.userHash = "MyHash";
  entry.userInfo.ip = "localhost";
  entry.userInfo.port = 12345;
  entry.userInfo.server = true;

  SourceInfo sourceInfo;
  sourceInfo.type = "SourceType";
  sourceInfo.tincId = "ProcessorId";
  sourceInfo.commandLineArguments = "args and args";

  SourceArgument arg_int8;
  arg_int8.id = "int8";
  arg_int8.setValue((int8_t)127);
  sourceInfo.arguments.push_back(arg_int8);

  SourceArgument arg_int16;
  arg_int16.id = "int16";
  arg_int16.setValue((int16_t)32767);
  sourceInfo.arguments.push_back(arg_int16);

  SourceArgument arg_int32;
  arg_int32.id = "int32";
  arg_int32.setValue((int32_t)2000000032);
  sourceInfo.arguments.push_back(arg_int32);

  SourceArgument arg_int64;
  arg_int64.id = "int64";
  arg_int64.setValue((int64_t)64000000064);
  sourceInfo.arguments.push_back(arg_int64);

  SourceArgument arg_uint8;
  arg_uint8.id = "uint8";
  arg_uint8.setValue((uint8_t)255);
  sourceInfo.arguments.push_back(arg_uint8);

  SourceArgument arg_uint16;
  arg_uint16.id = "uint16";
  arg_uint16.setValue((uint16_t)65535);
  sourceInfo.arguments.push_back(arg_uint16);

  SourceArgument arg_uint32;
  arg_uint32.id = "uint32";
  arg_uint32.setValue((uint32_t)3200000032);
  sourceInfo.arguments.push_back(arg_uint32);

  SourceArgument arg_uint64;
  arg_uint64.id = "uint64";
  arg_uint64.setValue((uint64_t)640000000064);
  sourceInfo.arguments.push_back(arg_uint64);

  SourceArgument arg_float;
  arg_float.id = "float";
  arg_float.setValue(4.05f);
  sourceInfo.arguments.push_back(arg_float);

  SourceArgument arg_double;
  arg_double.id = "double";
  arg_double.setValue(4.0512);
  sourceInfo.arguments.push_back(arg_double);

  SourceArgument arg_bool;
  arg_bool.id = "bool";
  arg_bool.setValue(true);
  sourceInfo.arguments.push_back(arg_bool);

  SourceArgument arg_string;
  arg_string.id = "string";
  arg_string.setValue(std::string("hello"));
  sourceInfo.arguments.push_back(arg_string);

  // Add dependencies
  sourceInfo.dependencies.push_back(arg_int8);
  sourceInfo.dependencies.push_back(arg_int16);
  sourceInfo.dependencies.push_back(arg_int32);
  sourceInfo.dependencies.push_back(arg_int64);
  sourceInfo.dependencies.push_back(arg_uint8);
  sourceInfo.dependencies.push_back(arg_uint16);
  sourceInfo.dependencies.push_back(arg_uint32);
  sourceInfo.dependencies.push_back(arg_uint64);
  sourceInfo.dependencies.push_back(arg_float);
  sourceInfo.dependencies.push_back(arg_double);
  sourceInfo.dependencies.push_back(arg_bool);
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
  EXPECT_EQ(entries[0].files.size(), 2);
  //  EXPECT_EQ(entries[0].files[0], "hello");
  //  EXPECT_EQ(entries[0].files[1], "world");
  EXPECT_TRUE(entries[0].stale);

  EXPECT_EQ(entries[0].userInfo.ip, "localhost");
  EXPECT_EQ(entries[0].userInfo.server, true);
  EXPECT_EQ(entries[0].userInfo.userName, "MyName");
  EXPECT_EQ(entries[0].userInfo.userHash, "MyHash");
  EXPECT_EQ(entries[0].userInfo.ip, "localhost");
  EXPECT_EQ(entries[0].userInfo.port, 12345);

  EXPECT_EQ(entries[0].sourceInfo.type, "SourceType");
  EXPECT_EQ(entries[0].sourceInfo.tincId, "ProcessorId");
  EXPECT_EQ(entries[0].sourceInfo.commandLineArguments, "args and args");

  EXPECT_EQ(entries[0].sourceInfo.arguments.size(), 12);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(0).id, "int8");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(0).getValue().type(),
            al::VariantType::VARIANT_INT8);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(0).getValue().get<int8_t>(),
            127);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(1).id, "int16");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(1).getValue().type(),
            al::VariantType::VARIANT_INT16);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(1).getValue().get<int16_t>(),
            32767);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(2).id, "int32");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(2).getValue().type(),
            al::VariantType::VARIANT_INT32);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(2).getValue().get<int32_t>(),
            2000000032);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(3).id, "int64");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(3).getValue().type(),
            al::VariantType::VARIANT_INT64);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(3).getValue().get<int64_t>(),
            64000000064);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(4).id, "uint8");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(4).getValue().type(),
            al::VariantType::VARIANT_UINT8);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(4).getValue().get<uint8_t>(),
            255);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(5).id, "uint16");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(5).getValue().type(),
            al::VariantType::VARIANT_UINT16);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(5).getValue().get<uint16_t>(),
            65535);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(6).id, "uint32");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(6).getValue().type(),
            al::VariantType::VARIANT_UINT32);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(6).getValue().get<uint32_t>(),
            3200000032);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(7).id, "uint64");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(7).getValue().type(),
            al::VariantType::VARIANT_UINT64);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(7).getValue().get<uint64_t>(),
            640000000064);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(8).id, "float");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(8).getValue().type(),
            al::VariantType::VARIANT_FLOAT);
  EXPECT_FLOAT_EQ(entries[0].sourceInfo.arguments.at(8).getValue().get<float>(),
                  4.05);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(9).id, "double");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(9).getValue().type(),
            al::VariantType::VARIANT_DOUBLE);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(9).getValue().get<double>(),
            4.0512);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(10).id, "bool");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(10).getValue().type(),
            al::VariantType::VARIANT_BOOL);
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(10).getValue().get<bool>(),
            true);

  EXPECT_EQ(entries[0].sourceInfo.arguments.at(11).id, "string");
  EXPECT_EQ(entries[0].sourceInfo.arguments.at(11).getValue().type(),
            al::VariantType::VARIANT_STRING);
  EXPECT_EQ(
      entries[0].sourceInfo.arguments.at(11).getValue().get<std::string>(),
      "hello");
  //
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(0).id, "int8");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(0).getValue().type(),
            al::VariantType::VARIANT_INT8);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(0).getValue().get<int8_t>(),
            127);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(1).id, "int16");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(1).getValue().type(),
            al::VariantType::VARIANT_INT16);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(1).getValue().get<int16_t>(),
            32767);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(2).id, "int32");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(2).getValue().type(),
            al::VariantType::VARIANT_INT32);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(2).getValue().get<int32_t>(),
            2000000032);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(3).id, "int64");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(3).getValue().type(),
            al::VariantType::VARIANT_INT64);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(3).getValue().get<int64_t>(),
            64000000064);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(4).id, "uint8");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(4).getValue().type(),
            al::VariantType::VARIANT_UINT8);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(4).getValue().get<uint8_t>(),
            255);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(5).id, "uint16");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(5).getValue().type(),
            al::VariantType::VARIANT_UINT16);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(5).getValue().get<uint16_t>(),
            65535);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(6).id, "uint32");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(6).getValue().type(),
            al::VariantType::VARIANT_UINT32);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(6).getValue().get<uint32_t>(),
            3200000032);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(7).id, "uint64");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(7).getValue().type(),
            al::VariantType::VARIANT_UINT64);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(7).getValue().get<uint64_t>(),
            640000000064);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(8).id, "float");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(8).getValue().type(),
            al::VariantType::VARIANT_FLOAT);
  EXPECT_FLOAT_EQ(
      entries[0].sourceInfo.dependencies.at(8).getValue().get<float>(), 4.05);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(9).id, "double");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(9).getValue().type(),
            al::VariantType::VARIANT_DOUBLE);
  EXPECT_DOUBLE_EQ(
      entries[0].sourceInfo.dependencies.at(9).getValue().get<double>(),
      4.0512);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(10).id, "bool");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(10).getValue().type(),
            al::VariantType::VARIANT_BOOL);
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(10).getValue().get<bool>(),
            true);

  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(11).id, "string");
  EXPECT_EQ(entries[0].sourceInfo.dependencies.at(11).getValue().type(),
            al::VariantType::VARIANT_STRING);
  EXPECT_EQ(
      entries[0].sourceInfo.dependencies.at(11).getValue().get<std::string>(),
      "hello");

  // TODO ML test all types for arguments and dependencies
}

TEST(Cache, ParameterSpace) {
  if (al::File::exists("cache/tinc_cache.json")) {
    al::File::remove("cache/tinc_cache.json");
  }

  // Make a ParameterSpace
  ParameterSpace ps;
  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);

  float values[5] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
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

  // Create a processor;
  ProcessorCpp processor("TincProcessor");
  processor.setOutputFileNames({"cache_ps.txt"});
  processor.processingFunction = [&]() {
    // Compute a value
    float value = processor.configuration["dim1"].get<float>() *
                      processor.configuration["dim2"].get<float>() +
                  processor.configuration["dim3"].get<float>();

    auto filenames = processor.getOutputFileNames();

    al::al_sleep(3); // Sleep for 3 seconds.
    // Write it to file
    std::ofstream f(filenames[0]);
    f << std::to_string(value);
    f.close();
    return true;
  };

  ps.enableCache("cache");

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
  ps.writeToNetCDF();
}

TEST(Cache, CRC) {

  // This test relies on data created by TEST(Cache, ParameterSpace)

  CacheManager cmanage(DistributedPath{"tinc_cache.json", "cache"});

  cmanage.updateFromDisk();

  ParameterSpace ps;
  ps.readFromNetCDF();

  // Create a processor;
  ProcessorCpp processor("TincProcessor");
  processor.setOutputFileNames({"cache_ps.txt"});
  processor.processingFunction = [&]() {
    // Compute a value
    float value = processor.configuration["dim1"].get<float>() *
                      processor.configuration["dim2"].get<float>() +
                  processor.configuration["dim3"].get<float>();

    auto filenames = processor.getOutputFileNames();

    al::al_sleep(3); // Sleep for 3 seconds.
    // Write it to file
    std::ofstream f(filenames[0]);
    f << std::to_string(value);
    f.close();
    return true;
  };

  auto dim1 = ps.getDimension("dim1");
  auto dim2 = ps.getDimension("dim2");
  auto dim3 = ps.getDimension("dim3");
  dim1->setCurrentValue(0.5);
  dim2->setCurrentValue(0.2);
  dim3->setCurrentValue(0.02);

  auto entry = ps.cacheEntryForProcessor(processor);

  auto cacheFiles = cmanage.findCache(entry.sourceInfo, false);

  EXPECT_EQ(cacheFiles.size(), 1);

  // Now write something different to the file, to simulate something
  // overwriting cache

  std::ofstream of("cache/" + cacheFiles[0]);
  of << "Dummy data";
  of.close();

  cacheFiles = cmanage.findCache(entry.sourceInfo);
  // There should be no matches as CRC, date and size check fails
  // What will happen when date is checked?
  EXPECT_EQ(cacheFiles.size(), 0);

  cacheFiles = cmanage.findCache(entry.sourceInfo, false);
  // There should be a matches as CRC is not checked
  EXPECT_EQ(cacheFiles.size(), 1);
}
