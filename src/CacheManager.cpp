#include "tinc/CacheManager.hpp"

#include <iostream>
#include <fstream>

#include "al/io/al_File.hpp"

#define TINC_META_VERSION_MAJOR 1
#define TINC_META_VERSION_MINOR 0

using namespace tinc;

// To regenerate this file run update_schema_cpp.sh
#include "tinc_cache_schema.cpp"

CacheManager::CacheManager(DistributedPath cachePath) : mCachePath(cachePath) {

  auto person_schema = nlohmann::json::parse(
      doc_tinc_cache_schema_json,
      doc_tinc_cache_schema_json + doc_tinc_cache_schema_json_len);

  try {
    mValidator.set_root_schema(person_schema); // insert root-schema
  } catch (const std::exception &e) {
    std::cerr << "Validation of schema failed, here is why: " << e.what()
              << "\n";
  }

  if (!al::File::exists(mCachePath.filePath())) {
    writeToDisk();
  } else {
    updateFromDisk();
  }
}

std::string CacheManager::cacheDirectory() { return mCachePath.path(); }

void CacheManager::updateFromDisk() {
  std::unique_lock<std::mutex> lk(mCacheLock);

  //  j["tincMetaVersionMajor"] = TINC_META_VERSION_MAJOR;
  //  j["tincMetaVersionMinor"] = TINC_META_VERSION_MINOR;
  //  j["entries"] = {};
  std::ifstream f(mCachePath.filePath());
  if (f.good()) {
    nlohmann::json j;
    f >> j;
    try {
      mValidator.validate(j);
    } catch (const std::exception &e) {
      std::cerr << "Validation failed, here is why: " << e.what() << std::endl;
      return;
    }
    if (j["tincMetaVersionMajor"] != TINC_META_VERSION_MAJOR ||
        j["tincMetaVersionMinor"] != TINC_META_VERSION_MINOR) {

      std::cerr << "Incompatible schema version: " << j["tincMetaVersionMajor"]
                << "." << j["tincMetaVersionMinor"] << " .This binary uses "
                << TINC_META_VERSION_MAJOR << "." << TINC_META_VERSION_MINOR
                << "\n";
      return;
    }
    mEntries.clear();
    for (auto entry : j["entries"]) {
      CacheEntry e;
      e.timestampStart = entry["timestamp"]["start"];
      e.timestampEnd = entry["timestamp"]["end"];
      e.filenames = entry["filenames"].get<std::vector<std::string>>();

      e.cacheHits = entry["cacheHits"];
      e.stale = entry["stale"];

      e.userInfo.userName = entry["userInfo"]["userName"];
      e.userInfo.userHash = entry["userInfo"]["userHash"];
      e.userInfo.ip = entry["userInfo"]["ip"];
      e.userInfo.port = entry["userInfo"]["port"];
      e.userInfo.server = entry["userInfo"]["server"];

      e.sourceInfo.type = entry["sourceInfo"]["type"];
      e.sourceInfo.commandLineArguments =
          entry["sourceInfo"]["commandLineArguments"];
      e.sourceInfo.hash = entry["sourceInfo"]["hash"];

      for (auto arg : entry["sourceInfo"]["arguments"]) {
        SourceArgument newArg;
        newArg.id = arg["id"];
        if (arg["value"].is_number_float()) {
          newArg.value = arg["value"].get<double>();
        } else if (arg["value"].is_number_integer()) {
          newArg.value = arg["value"].get<int64_t>();
        } else if (arg["value"].is_string()) {
          newArg.value = arg["value"].get<std::string>();
        }
        e.sourceInfo.arguments.push_back(newArg);
      }
      for (auto arg : entry["sourceInfo"]["dependencies"]) {
        SourceArgument newArg;
        newArg.id = arg["id"];
        if (arg["value"].is_number_float()) {
          newArg.value = arg["value"].get<double>();
        } else if (arg["value"].is_number_integer()) {
          newArg.value = arg["value"].get<int64_t>();
        } else if (arg["value"].is_string()) {
          newArg.value = arg["value"].get<std::string>();
        }
        e.sourceInfo.dependencies.push_back(newArg);
      }
      mEntries.push_back(e);
    }

  } else {
    std::cerr << "Error attempting to read cache: " << mCachePath.filePath()
              << std::endl;
  }
}

// when creating the validator

void CacheManager::writeToDisk() {
  std::unique_lock<std::mutex> lk(mCacheLock);
  std::ofstream o(mCachePath.filePath());
  if (o.good()) {
    nlohmann::json j;
    j["tincMetaVersionMajor"] = TINC_META_VERSION_MAJOR;
    j["tincMetaVersionMinor"] = TINC_META_VERSION_MINOR;
    j["entries"] = std::vector<nlohmann::json>();

    for (auto &e : mEntries) {
      nlohmann::json entry;
      entry["timestamp"]["start"] = e.timestampStart;
      entry["timestamp"]["end"] = e.timestampEnd;
      entry["filenames"] = e.filenames;

      entry["cacheHits"] = e.cacheHits;
      entry["stale"] = e.stale;

      entry["userInfo"]["userName"] = e.userInfo.userName;
      entry["userInfo"]["userHash"] = e.userInfo.userHash;
      entry["userInfo"]["ip"] = e.userInfo.ip;
      entry["userInfo"]["port"] = e.userInfo.port;
      entry["userInfo"]["server"] = e.userInfo.server;

      entry["sourceInfo"]["type"] = e.sourceInfo.type;
      entry["sourceInfo"]["commandLineArguments"] =
          e.sourceInfo.commandLineArguments;
      entry["sourceInfo"]["hash"] = e.sourceInfo.hash;
      entry["sourceInfo"]["arguments"] = std::vector<nlohmann::json>();
      entry["sourceInfo"]["dependencies"] = std::vector<nlohmann::json>();
      for (auto arg : e.sourceInfo.arguments) {
        nlohmann::json newArg;
        newArg["id"] = arg.id;
        if (arg.value.type == VARIANT_DOUBLE ||
            arg.value.type == VARIANT_FLOAT) {
          newArg["value"] = arg.value.valueDouble;
        } else if (arg.value.type == VARIANT_INT32 ||
                   arg.value.type == VARIANT_INT64) {
          newArg["value"] = arg.value.valueInt64;
        } else if (arg.value.type == VARIANT_STRING) {
          newArg["value"] = arg.value.valueStr;
        } else {
          newArg["value"] = nlohmann::json();
        }
        entry["sourceInfo"]["arguments"].push_back(newArg);
      }
      for (auto arg : e.sourceInfo.dependencies) {
        nlohmann::json newArg;
        newArg["id"] = arg.id;
        if (arg.value.type == VARIANT_DOUBLE ||
            arg.value.type == VARIANT_FLOAT) {
          newArg["value"] = arg.value.valueDouble;
        } else if (arg.value.type == VARIANT_INT32 ||
                   arg.value.type == VARIANT_INT64) {
          newArg["value"] = arg.value.valueInt64;
        } else if (arg.value.type == VARIANT_STRING) {
          newArg["value"] = arg.value.valueStr;
        } else {
          newArg["value"] = nlohmann::json();
        }
        entry["sourceInfo"]["dependencies"].push_back(newArg);
      }

      j["entries"].push_back(entry);
    }
    o << j << std::endl;
  } else {
    std::cerr << "ERROR: Can't create cache file: " << mCachePath.filePath()
              << std::endl;
    throw std::runtime_error("Can't create cache file");
  }
}

void CacheManager::tinc_schema_format_checker(const std::string &format,
                                              const std::string &value) {
  if (format == "date-time") {
    // TODO validate date time
    return;
    //    throw std::invalid_argument("value is not a good something");
  } else
    throw std::logic_error("Don't know how to validate " + format);
}