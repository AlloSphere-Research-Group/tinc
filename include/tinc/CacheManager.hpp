#ifndef CACHEMANAGER_HPP
#define CACHEMANAGER_HPP

#include "nlohmann/json-schema.hpp"
#include "nlohmann/json.hpp"

#include "al/types/al_VariantValue.hpp"
#include "tinc/DistributedPath.hpp"

#include <cinttypes>
#include <mutex>
#include <string>
#include <vector>

namespace tinc {

struct UserInfo {
  std::string userName;
  std::string userHash;
  std::string ip;
  uint16_t port;
  bool server{false};
};

class SourceArgument {
public:
  SourceArgument() {}
  SourceArgument(const SourceArgument &src) {
    id = src.id;
    copyValueFromSource(*this, src);
  }
  SourceArgument &operator=(const SourceArgument &other) {
    if (this != &other) // not a self-assignment
    {
      this->id = other.id;
      copyValueFromSource(*this, other);
    }
    return *this;
  }
  // Move constructor
  SourceArgument(SourceArgument &&that) noexcept { swap(*this, that); }

  // Move assignment operator
  SourceArgument &operator=(SourceArgument &&that) {
    swap(*this, that);
    return *this;
  }

  friend void swap(SourceArgument &lhs, SourceArgument &rhs) noexcept {
    std::swap(lhs.value, rhs.value);
    std::swap(lhs.id, rhs.id);
  }

  static void copyValueFromSource(SourceArgument &dst,
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
    }
  }

  template <typename T> void setValue(T value_) {
    value = std::make_unique<al::VariantValue>(value_);
  }
  std::string id;

  al::VariantValue getValue() const;

private:
  std::unique_ptr<al::VariantValue> value;
};

struct FileDependency {
  DistributedPath file;
  std::string modified;
  uint64_t size;
};

class SourceInfo {
public:
  SourceInfo() {}
  SourceInfo(const SourceInfo &src) { copyValueFromSource(*this, src); }
  SourceInfo &operator=(const SourceInfo &other) {
    if (this != &other) // not a self-assignment
    {
      copyValueFromSource(*this, other);
    }
    return *this;
  }

  static void copyValueFromSource(SourceInfo &dst, const SourceInfo &src) {

    dst.type = src.type;
    dst.tincId = src.tincId;
    dst.commandLineArguments = src.commandLineArguments;
    dst.workingPath = src.workingPath;
    dst.hash = src.hash;
    for (auto &arg : src.arguments) {
      dst.arguments.push_back(arg);
    }
    for (auto &dep : src.dependencies) {
      dst.dependencies.push_back(dep);
    }
    dst.fileDependencies = src.fileDependencies;
  }

  // Write copy/move constructor. Write copy operator
  std::string type;
  std::string
      tincId; // TODO add heuristics to match source even if id has changed.
  std::string commandLineArguments;
  DistributedPath workingPath{""};
  std::string hash;
  std::vector<SourceArgument> arguments;
  std::vector<SourceArgument> dependencies;
  std::vector<FileDependency> fileDependencies;
};

struct CacheEntry {
  std::string timestampStart;
  std::string timestampEnd;
  std::vector<std::string> filenames;
  UserInfo userInfo;
  SourceInfo sourceInfo;
  uint64_t cacheHits{0};
  bool stale{false};
};

/**
 * @brief The CacheManager class
 */
class CacheManager {
public:
  CacheManager(DistributedPath cachePath = DistributedPath("tinc_cache.json"));

  /**
   * @brief append cache entry
   * @param the CacheEntry entry
   */
  void appendEntry(CacheEntry &entry);

  /**
   * @brief Get all in memory entries
   * @param count Number of most recent entries to get
   * @return vector of CacheEntry objects
   *
   * If count is 0, or greater than the number of entries, all the entries are
   * returned.
   */
  std::vector<CacheEntry> entries(size_t count = 0);

  std::vector<std::string> findCache(const SourceInfo &sourceInfo,
                                     bool verifyHash = true);
  /**
   * @brief Clear all cached files, and cache information.
   *
   * Although this function will only delete files listed in the cache file, it
   * should be used with caution.
   */
  void clearCache();

  /**
   * @brief Get full cache path
   */
  std::string cacheDirectory();

  /**
   * @brief Read and validate cache file from disk
   *
   * This replaces the current in memory cache, so make sure you call
   * writeToDisk() first if you want to store in memory cache.
   */
  void updateFromDisk();

  /**
   * @brief Write the current in memory cache to disk
   *
   * This will overwrite the cache metadata file on disk
   */
  void writeToDisk();

  /**
   * @brief Read cache metadata
   * @return the json metadata as a string
   */
  std::string dump();

protected:
  DistributedPath mCachePath;
  std::mutex mCacheLock;

  // In memory cache
  std::vector<CacheEntry> mEntries;

  // Function to add validators for special types like date-time
  static void tincSchemaFormatChecker(const std::string &format,
                                      const std::string &value);

  nlohmann::json_schema::json_validator mValidator{nullptr,
                                                   tincSchemaFormatChecker};
};
} // namespace tinc

#endif // CACHEMANAGER_HPP
