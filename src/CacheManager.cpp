#include "tinc/CacheManager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

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
  if (!al::File::exists(mCachePath.rootPath + mCachePath.relativePath)) {
    al::Dir::make(mCachePath.rootPath + mCachePath.relativePath);
  }

  if (!al::File::exists(mCachePath.filePath())) {
    writeToDisk();
  } else {
    try {
      updateFromDisk();
    } catch (std::exception &e) {
      std::string backupFilename = mCachePath.filePath() + ".old";
      size_t count = 0;
      while (al::File::exists(mCachePath.filePath() + std::to_string(count))) {
        count++;
      }
      if (!al::File::copy(mCachePath.filePath(),
                          mCachePath.filePath() + std::to_string(count))) {
        std::cerr << "Cache invalid and backup failed." << std::endl;
        throw std::exception();
      }
      std::cerr << "Invalid cache format. Ignoring old cache." << std::endl;
    }
  }
}

void CacheManager::appendEntry(CacheEntry &entry) {
  std::unique_lock<std::mutex> lk(mCacheLock);
  mEntries.push_back(entry);
}

std::vector<std::string> CacheManager::findCache(const SourceInfo &sourceInfo,
                                                 bool verifyHash) {
  std::unique_lock<std::mutex> lk(mCacheLock);
  for (const auto &entry : mEntries) {
    if (entry.sourceInfo.commandLineArguments ==
            sourceInfo.commandLineArguments &&
        entry.sourceInfo.tincId == sourceInfo.tincId &&
        entry.sourceInfo.type == sourceInfo.type) {
      auto entryArguments = entry.sourceInfo.arguments;
      bool argsMatch = false;
      if (sourceInfo.arguments.size() == entry.sourceInfo.arguments.size()) {
        size_t matchCount = 0;
        for (const auto &sourceArg : sourceInfo.arguments) {
          for (auto arg = entryArguments.begin(); arg != entryArguments.end();
               arg++) {
            if (sourceArg.id == arg->id) {
              if (sourceArg.value.type == arg->value.type) {
                if (sourceArg.value.type == VARIANT_DOUBLE ||
                    sourceArg.value.type == VARIANT_FLOAT) {
                  if (sourceArg.value.valueDouble == arg->value.valueDouble) {
                    entryArguments.erase(arg);
                    matchCount++;
                    break;
                  }
                } else if (sourceArg.value.type == VARIANT_INT32 ||
                           sourceArg.value.type == VARIANT_INT64) {
                  if (sourceArg.value.valueInt64 == arg->value.valueInt64) {
                    entryArguments.erase(arg);
                    matchCount++;
                    break;
                  }
                } else if (sourceArg.value.type == VARIANT_STRING) {
                  if (sourceArg.value.valueStr == arg->value.valueStr) {
                    entryArguments.erase(arg);
                    matchCount++;
                    break;
                  }
                } else {
                  std::cerr << "ERROR: Unsupported type for argument value"
                            << std::endl;
                }
              } else {
                std::cout << "ERROR: type mismatch for argument in cache. "
                             "Ignoring cache"
                          << std::endl;
                continue;
              }
            }
          }
        }
        if (matchCount == sourceInfo.arguments.size() &&
            entryArguments.size() == 0) {
          return entry.filenames;
        }
      } else {
        // TODO develop mechanisms to recover stale cache
        std::cout << "Warning, cache entry found, but argument size mismatch"
                  << std::endl;
      }
    }
  }
  return {};
}

std::string CacheManager::cacheDirectory() {

  if (!al::File::isDirectory(mCachePath.path())) {
    al::Dir::make(mCachePath.path());
  }
  return mCachePath.path();
}

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
      e.sourceInfo.tincId = entry["sourceInfo"]["tincId"];
      e.sourceInfo.commandLineArguments =
          entry["sourceInfo"]["commandLineArguments"];

      e.sourceInfo.workingPath.relativePath =
          entry["sourceInfo"]["workingPath"]["relativePath"];
      e.sourceInfo.workingPath.rootPath =
          entry["sourceInfo"]["workingPath"]["rootPath"];
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
      for (auto arg : entry["sourceInfo"]["fileDependencies"]) {
        FileDependency fileDep;
        fileDep.file = DistributedPath(arg["file"]["filename"],
                                       arg["file"]["relativePath"],
                                       arg["file"]["rootPath"]);
        fileDep.modified = arg["modified"];
        fileDep.size = arg["size"];

        e.sourceInfo.fileDependencies.push_back(fileDep);
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
      entry["sourceInfo"]["tincId"] = e.sourceInfo.tincId;
      entry["sourceInfo"]["commandLineArguments"] =
          e.sourceInfo.commandLineArguments;

      // TODO validate working path
      entry["sourceInfo"]["workingPath"]["relativePath"] =
          e.sourceInfo.workingPath.relativePath;
      entry["sourceInfo"]["workingPath"]["rootPath"] =
          e.sourceInfo.workingPath.rootPath;
      entry["sourceInfo"]["hash"] = e.sourceInfo.hash;
      entry["sourceInfo"]["arguments"] = std::vector<nlohmann::json>();
      entry["sourceInfo"]["dependencies"] = std::vector<nlohmann::json>();
      entry["sourceInfo"]["fileDependencies"] = std::vector<nlohmann::json>();
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
      for (auto arg : e.sourceInfo.fileDependencies) {
        nlohmann::json newArg;
        newArg["file"]["filename"] = arg.file.filename;
        newArg["file"]["relativePath"] = arg.file.relativePath;
        newArg["file"]["rootPath"] = arg.file.rootPath;
        newArg["modified"] = "";
        newArg["size"] = 0; // TODO write modified and size values
        entry["sourceInfo"]["fileDependencies"].push_back(newArg);
      }

      j["entries"].push_back(entry);
    }
    o << j << std::endl;
    o.close();
  } else {
    std::cerr << "ERROR: Can't create cache file: " << mCachePath.filePath()
              << std::endl;
    throw std::runtime_error("Can't create cache file");
  }
}

std::string CacheManager::dump() {
  writeToDisk();
  std::unique_lock<std::mutex> lk(mCacheLock);
  std::ifstream f(mCachePath.filePath());
  std::stringstream ss;

  ss << f.rdbuf();
  return ss.str();
}

void CacheManager::tincSchemaFormatChecker(const std::string &format,
                                           const std::string &value) {
  if (format == "date-time") {
    // TODO validate date time
    return;
    //    throw std::invalid_argument("value is not a good something");
  } else
    throw std::logic_error("Don't know how to validate " + format);
}
