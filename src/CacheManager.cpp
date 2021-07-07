#include "tinc/CacheManager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "al/io/al_File.hpp"

#define STB_DEFINE
#include "stb.h"

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
    } catch (std::exception & /*e*/) {
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

int32_t CacheManager::computeCrc32(std::string filename) {
  std::ifstream is(filename, std::ifstream::binary);
  uint32_t crc = 0;
  if (is) {
    // get length of file:
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    int toRead = length > 8192 ? 8192 : length;
    while (length > 0) {
      char buffer[8192];
      is.read(buffer, toRead);
      crc = stb_crc32_block(crc, (unsigned char *)buffer, toRead);
      length -= toRead;
      if (length < toRead) {
        toRead = length;
      }
    }
  } else {
    // report error
  }
  return crc;
}

void CacheManager::appendEntry(CacheEntry &entry) {
  std::unique_lock<std::mutex> lk(mCacheLock);
  mEntries.push_back(entry);
}

std::vector<CacheEntry> CacheManager::entries(size_t count) {
  std::unique_lock<std::mutex> lk(mCacheLock);
  if (count == 0 || count >= mEntries.size()) {
    return mEntries;
  }
  std::vector<CacheEntry> e;
  size_t offset = mEntries.size() - count;
  e.insert(e.begin(), mEntries.begin() + offset, mEntries.end());
  return e;
}

// TODO move to allolib
bool valueMatch(const al::VariantValue &v1, const al::VariantValue &v2) {
  if (v1.type() == v2.type()) {
    if (v1.type() == al::VariantType::VARIANT_FLOAT) {
      if (v1.get<float>() == v2.get<float>()) {
        return true;
      }
    } else if (v1.type() == al::VariantType::VARIANT_DOUBLE) {
      if (v1.get<double>() == v2.get<double>()) {
        return true;
      }
    } else if (v1.type() == al::VariantType::VARIANT_INT32) {
      if (v1.get<int32_t>() == v2.get<int32_t>()) {
        return true;
      }
    } else if (v1.type() == al::VariantType::VARIANT_INT64) {
      if (v1.get<int64_t>() == v2.get<int64_t>()) {
        return true;
      }
    } else if (v1.type() == al::VariantType::VARIANT_STRING) {
      if (v1.get<std::string>() == v2.get<std::string>()) {
        return true;
      }
    } else {
      std::cerr << "ERROR: Unsupported type for argument value" << std::endl;
    }
  }
  return false;
}

std::vector<std::string>
CacheManager::findCache(const SourceInfo &querySourceInfo, bool validateFile) {
  std::unique_lock<std::mutex> lk(mCacheLock);
  size_t counter = 0;
  for (const auto &existingEntry : mEntries) {
    if (existingEntry.sourceInfo.commandLineArguments ==
            querySourceInfo.commandLineArguments &&
        existingEntry.sourceInfo.tincId == querySourceInfo.tincId &&
        existingEntry.sourceInfo.type == querySourceInfo.type) {
      auto &entryArguments = existingEntry.sourceInfo.arguments;
      auto &entryDependencies = existingEntry.sourceInfo.dependencies;
      if (querySourceInfo.arguments.size() ==
              existingEntry.sourceInfo.arguments.size() &&
          querySourceInfo.dependencies.size() ==
              existingEntry.sourceInfo.dependencies.size()) {
        size_t matchCount = 0;
        for (const auto &querySourceArg : querySourceInfo.arguments) {
          for (auto arg = entryArguments.begin(); arg != entryArguments.end();
               arg++) {
            if (querySourceArg.id == arg->id) {
              if (valueMatch(arg->getValue(), querySourceArg.getValue())) {
                //                entryArguments.erase(arg);
                matchCount++;
                break;
              }
            } /*else {
              std::cout << "ERROR: argument in cache. "
                           "Ignoring cache"
                        << std::endl;
              continue;
            }*/
          }
        }
        for (const auto &sourceDep : querySourceInfo.dependencies) {
          for (auto arg = entryDependencies.begin();
               arg != entryDependencies.end(); arg++) {

            if (sourceDep.id == arg->id) {
              if (valueMatch(arg->getValue(), sourceDep.getValue())) {
                //                entryArguments.erase(arg);
                matchCount++;
                break;
              }
            }
          }
        }
        if (matchCount == querySourceInfo.arguments.size() +
                              querySourceInfo.dependencies.size()) {
          std::vector<std::string> files;
          for (auto fentry : existingEntry.files) {
            // TODO ML do CRC, data and size check
            if (validateFile) {
              std::string filePath =
                  mCachePath.path() + "/" + fentry.file.filePath();
              if (!std::filesystem::exists(filePath)) {
                std::cerr << "ERROR cached file missing: " << filePath
                          << std::endl;
                continue;
              }
              auto modifiedTime = std::filesystem::last_write_time(filePath);
              std::time_t cftime = __tinc_to_time_t(modifiedTime);
              std::stringstream ss;
              ss << std::put_time(std::localtime(&cftime), "%FT%T%z");

              uint64_t size = std::filesystem::file_size(filePath);

              uint32_t crc = computeCrc32(filePath);
              auto cachedCRC = std::stoul(fentry.hash);
              if (fentry.modified != ss.str()) {
                // FIXME if there is a mismatch, this will preclude this entry
                // from being cached.
                std::cerr << "ERROR modified mismatch " << ss.str()
                          << std::endl;
                continue;
              }
              if (fentry.size != size) {
                std::cerr << "ERROR size mismatch" << std::endl;
                continue;
              }
              if (cachedCRC != crc) {
                std::cerr << "ERROR CRC mismatch" << std::endl;
                continue;
              }
              files.push_back(fentry.file.filePath());
            } else {
              files.push_back(fentry.file.filePath());
            }
          }
          return files;
        }
      } else {
        // TODO develop mechanisms to recover stale cache
        std::cout << "Warning, cache entry found, but argument size mismatch"
                  << std::endl;
      }
    }
    counter++;
  }
  return {};
}

void CacheManager::clearCache() {
  std::unique_lock<std::mutex> lk(mCacheLock);
  updateFromDisk();
  for (auto &entry : mEntries) {
    for (auto file : entry.files) {
      auto name = mCachePath.path() + file.file.filePath();
      // TODO ML check CRC and size before removing
      if (al::File::remove(name)) {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " Error removing cache file: " << name << std::endl;
      }
    }
  }
  mEntries.clear();
  writeToDisk();
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
    try {
      f >> j;
    } catch (const std::exception &e) {
      std::cerr << "Cache json parsing failed: " << e.what() << std::endl;
      return;
    }
    try {
      mValidator.validate(j);
    } catch (const std::exception &e) {
      std::cerr << "Cache json validation failed: " << e.what() << std::endl;
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
      e.files = {};

      for (auto f : entry["files"]) {
        e.files.emplace_back(FileDependency{
            DistributedPath(f["file"]["filename"], f["file"]["relativePath"],
                            f["file"]["rootPath"], f["file"]["protocolId"]),
            f["modified"], f["size"], f["hash"]});
      }

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

      for (auto &arg : entry["sourceInfo"]["arguments"]) {
        SourceArgument newArg;
        newArg.id = arg["id"];
        if (arg.find("nctype") !=
            arg.end()) { // TODO: Temporary should be removed for release
          if (arg["nctype"] == 0) {
            if (arg["value"].is_number_float()) {
              newArg.setValue(arg["value"].get<double>());
            } else if (arg["value"].is_number_integer()) {
              newArg.setValue(arg["value"].get<int64_t>());
            } else if (arg["value"].is_string()) {
              newArg.setValue(arg["value"].get<std::string>());
            }
          } else {
            if (arg["nctype"] > 0) {
              switch ((al::VariantType)arg["nctype"]) {
              case al::VariantType::VARIANT_INT64:
                newArg.setValue(arg["value"].get<int64_t>());
                break;
              case al::VariantType::VARIANT_INT32:
                newArg.setValue(arg["value"].get<int32_t>());
                break;
              case al::VariantType::VARIANT_INT16:
                newArg.setValue(arg["value"].get<int16_t>());
                break;
              case al::VariantType::VARIANT_INT8:
                newArg.setValue(arg["value"].get<int8_t>());
                break;
              case al::VariantType::VARIANT_UINT64:
                newArg.setValue(arg["value"].get<uint64_t>());
                break;
              case al::VariantType::VARIANT_UINT32:
                newArg.setValue(arg["value"].get<uint32_t>());
                break;
              case al::VariantType::VARIANT_UINT16:
                newArg.setValue(arg["value"].get<uint16_t>());
                break;
              case al::VariantType::VARIANT_UINT8:
                newArg.setValue(arg["value"].get<uint8_t>());
                break;
              case al::VariantType::VARIANT_DOUBLE:
                newArg.setValue(arg["value"].get<double>());
                break;
              case al::VariantType::VARIANT_FLOAT:
                newArg.setValue(arg["value"].get<float>());
                break;
              case al::VariantType::VARIANT_STRING:
                newArg.setValue(arg["value"].get<std::string>());
                break;
              case al::VariantType::VARIANT_BOOL:
                newArg.setValue(arg["value"].get<bool>());
                break;
              case al::VariantType::VARIANT_NONE:
                break;
              default:
                break;
              }

            } else {
              std::cerr << "Invalid type for argument " << arg["nctype"]
                        << std::endl;
            }
          }
          e.sourceInfo.arguments.push_back(std::move(newArg));
        }
      }
      for (auto &arg : entry["sourceInfo"]["dependencies"]) {
        SourceArgument newArg;
        newArg.id = arg["id"];
        if (arg.find("nctype") !=
            arg.end()) { // TODO: Temporary should be removed for release
          if (arg["nctype"] == 0) {
            if (arg["value"].is_number_float()) {
              newArg.setValue(arg["value"].get<double>());
            } else if (arg["value"].is_number_integer()) {
              newArg.setValue(arg["value"].get<int64_t>());
            } else if (arg["value"].is_string()) {
              newArg.setValue(arg["value"].get<std::string>());
            }
          } else {
            if (arg["nctype"] > 0) {
              switch ((al::VariantType)arg["nctype"]) {
              case al::VariantType::VARIANT_INT64:
                newArg.setValue(arg["value"].get<int64_t>());
                break;
              case al::VariantType::VARIANT_INT32:
                newArg.setValue(arg["value"].get<int32_t>());
                break;
              case al::VariantType::VARIANT_INT16:
                newArg.setValue(arg["value"].get<int16_t>());
                break;
              case al::VariantType::VARIANT_INT8:
                newArg.setValue(arg["value"].get<int8_t>());
                break;
              case al::VariantType::VARIANT_UINT64:
                newArg.setValue(arg["value"].get<uint64_t>());
                break;
              case al::VariantType::VARIANT_UINT32:
                newArg.setValue(arg["value"].get<uint32_t>());
                break;
              case al::VariantType::VARIANT_UINT16:
                newArg.setValue(arg["value"].get<uint16_t>());
                break;
              case al::VariantType::VARIANT_UINT8:
                newArg.setValue(arg["value"].get<uint8_t>());
                break;
              case al::VariantType::VARIANT_DOUBLE:
                newArg.setValue(arg["value"].get<double>());
                break;
              case al::VariantType::VARIANT_FLOAT:
                newArg.setValue(arg["value"].get<float>());
                break;
              case al::VariantType::VARIANT_BOOL:
                newArg.setValue(arg["value"].get<bool>());
                break;
              case al::VariantType::VARIANT_STRING:
                newArg.setValue(arg["value"].get<std::string>());
                break;
              case al::VariantType::VARIANT_NONE:
                break;
              default:
                break;
              }

            } else {
              std::cerr << "Invalid type for dependency " << arg["nctype"]
                        << std::endl;
            }
          }
          e.sourceInfo.dependencies.push_back(std::move(newArg));
        }
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

void CacheManager::writeToDisk() {
  std::unique_lock<std::mutex> lk(mCacheLock);
  if (al::File::exists(mCachePath.filePath())) {
    // Keep backup in case writing metadata fails.
    if (al::File::exists(mCachePath.filePath() + ".bak")) {
      al::File::remove(mCachePath.filePath() + ".bak");
    }
    al::File::copy(mCachePath.filePath(), mCachePath.filePath() + ".bak");
  }
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
      entry["files"] = {};

      for (auto f : e.files) {
        nlohmann::json fileObj;

        fileObj["file"] = {};
        fileObj["file"]["relativePath"] = f.file.relativePath;
        fileObj["file"]["rootPath"] = f.file.rootPath;
        fileObj["file"]["filename"] = f.file.filename;
        fileObj["file"]["protocolId"] = f.file.protocolId;
        fileObj["hash"] = f.hash;
        fileObj["modified"] = f.modified;
        fileObj["size"] = f.size;
        entry["files"].push_back(fileObj);
      }

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
      entry["sourceInfo"]["arguments"] = std::vector<nlohmann::json>();
      entry["sourceInfo"]["dependencies"] = std::vector<nlohmann::json>();
      entry["sourceInfo"]["fileDependencies"] = std::vector<nlohmann::json>();
      for (auto &arg : e.sourceInfo.arguments) {
        nlohmann::json newArg;
        newArg["id"] = arg.id;
        newArg["nctype"] = arg.getValue().type();
        // TODO ML support all types. Done.
        if (arg.getValue().type() == al::VariantType::VARIANT_DOUBLE) {
          newArg["value"] = arg.getValue().get<double>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_FLOAT) {
          newArg["value"] = arg.getValue().get<float>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT8) {
          newArg["value"] = arg.getValue().get<int8_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT8) {
          newArg["value"] = arg.getValue().get<uint8_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT16) {
          newArg["value"] = arg.getValue().get<int16_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT16) {
          newArg["value"] = arg.getValue().get<uint16_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT32) {
          newArg["value"] = arg.getValue().get<int32_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT32) {
          newArg["value"] = arg.getValue().get<uint32_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT64) {
          newArg["value"] = arg.getValue().get<int64_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT64) {
          newArg["value"] = arg.getValue().get<uint64_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_CHAR) {
          newArg["value"] = arg.getValue().get<char>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_BOOL) {
          newArg["value"] = arg.getValue().get<bool>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_STRING) {
          newArg["value"] = arg.getValue().get<std::string>();
        } else {
          newArg["value"] = nlohmann::json();
        }
        entry["sourceInfo"]["arguments"].push_back(newArg);
      }
      for (auto &arg : e.sourceInfo.dependencies) {
        nlohmann::json newArg;
        newArg["id"] = arg.id;
        // TODO ML support all types. Done.
        newArg["nctype"] = arg.getValue().type();
        if (arg.getValue().type() == al::VariantType::VARIANT_DOUBLE) {
          newArg["value"] = arg.getValue().get<double>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_FLOAT) {
          newArg["value"] = arg.getValue().get<float>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT8) {
          newArg["value"] = arg.getValue().get<int8_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT8) {
          newArg["value"] = arg.getValue().get<uint8_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT16) {
          newArg["value"] = arg.getValue().get<int16_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT16) {
          newArg["value"] = arg.getValue().get<uint16_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT32) {
          newArg["value"] = arg.getValue().get<int32_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT32) {
          newArg["value"] = arg.getValue().get<uint32_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_INT64) {
          newArg["value"] = arg.getValue().get<int64_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_UINT64) {
          newArg["value"] = arg.getValue().get<uint64_t>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_BOOL) {
          newArg["value"] = arg.getValue().get<bool>();
        } else if (arg.getValue().type() == al::VariantType::VARIANT_STRING) {
          newArg["value"] = arg.getValue().get<std::string>();
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
        newArg["modified"] = arg.modified;
        newArg["size"] = arg.size;
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

SourceArgument::SourceArgument(const SourceArgument &src) {
  id = src.id;
  copyValueFromSource(*this, src);
}

SourceArgument &SourceArgument::operator=(const SourceArgument &other) {
  if (this != &other) // not a self-assignment
  {
    this->id = other.id;
    copyValueFromSource(*this, other);
  }
  return *this;
}

void SourceArgument::copyValueFromSource(SourceArgument &dst,
                                         const SourceArgument &src) {
  switch (src.getValue().type()) {
  case al::VariantType::VARIANT_NONE:
    //      dst.setValue();
    break;
  case al::VariantType::VARIANT_INT64:
    dst.setValue(src.getValue().get<int64_t>());
    break;
  case al::VariantType::VARIANT_INT32:
    dst.setValue(src.getValue().get<int32_t>());
    break;
  case al::VariantType::VARIANT_INT16:
    dst.setValue(src.getValue().get<int16_t>());
    break;
  case al::VariantType::VARIANT_INT8:
    dst.setValue(src.getValue().get<int8_t>());
    break;
  case al::VariantType::VARIANT_UINT64:
    dst.setValue(src.getValue().get<uint64_t>());
    break;
  case al::VariantType::VARIANT_UINT32:
    dst.setValue(src.getValue().get<uint32_t>());
    break;
  case al::VariantType::VARIANT_UINT16:
    dst.setValue(src.getValue().get<uint16_t>());
    break;
  case al::VariantType::VARIANT_UINT8:
    dst.setValue(src.getValue().get<uint8_t>());
    break;
  case al::VariantType::VARIANT_DOUBLE:
    dst.setValue(src.getValue().get<double>());
    break;
  case al::VariantType::VARIANT_FLOAT:
    dst.setValue(src.getValue().get<float>());
    break;
  case al::VariantType::VARIANT_STRING:
    dst.setValue(src.getValue().get<std::string>());
    break;
  case al::VariantType::VARIANT_CHAR:
    dst.setValue(src.getValue().get<char>());
    break;
  case al::VariantType::VARIANT_BOOL:
    dst.setValue(src.getValue().get<bool>());
    break;
  }
}

SourceArgument::SourceArgument(SourceArgument &&that) noexcept {
  swap(*this, that);
}

SourceArgument &SourceArgument::operator=(SourceArgument &&that) {
  swap(*this, that);
  return *this;
}

al::VariantValue SourceArgument::getValue() const {
  if (value) {
    return *value;
  } else {
    return al::VariantValue();
  }
}

FileDependency &FileDependency::operator=(const FileDependency &other) {
  if (this != &other) // not a self-assignment
  {
    copyValueFromSource(*this, other);
  }
  return *this;
}

void FileDependency::copyValueFromSource(FileDependency &dst,
                                         const FileDependency &src) {
  dst.file = src.file;
  dst.modified = src.modified;
  dst.size = src.size;
  dst.hash = src.hash;
}

void SourceInfo::copyValueFromSource(SourceInfo &dst, const SourceInfo &src) {

  dst.type = src.type;
  dst.tincId = src.tincId;
  dst.commandLineArguments = src.commandLineArguments;
  dst.workingPath = src.workingPath;
  dst.arguments.clear();
  for (auto &arg : src.arguments) {
    dst.arguments.push_back(arg);
  }
  dst.dependencies.clear();
  for (auto &dep : src.dependencies) {
    dst.dependencies.push_back(dep);
  }
  dst.fileDependencies = src.fileDependencies;
}
