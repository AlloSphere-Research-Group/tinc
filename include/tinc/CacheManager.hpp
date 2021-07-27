#ifndef CACHEMANAGER_HPP
#define CACHEMANAGER_HPP

#include "nlohmann/json-schema.hpp"
#include "nlohmann/json.hpp"

#include "al/types/al_VariantValue.hpp"
#include "tinc/DistributedPath.hpp"

#include <chrono>
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
  SourceArgument(const SourceArgument &src);
  SourceArgument &operator=(const SourceArgument &other);
  SourceArgument(SourceArgument &&that) noexcept;   // Move constructor
  SourceArgument &operator=(SourceArgument &&that); // Move assignment operator

  void setValue(bool value_) {
    value = std::make_unique<al::VariantValue>(value_);
  }

  template <typename T> void setValue(T value_) {
    value = std::make_unique<al::VariantValue>(value_);
  }
  al::VariantValue getValue() const;

  std::string id;

private:
  static void copyValueFromSource(SourceArgument &dst,
                                  const SourceArgument &src);
  friend void swap(SourceArgument &lhs, SourceArgument &rhs) noexcept {
    std::swap(lhs.value, rhs.value);
    std::swap(lhs.id, rhs.id);
  }
  std::unique_ptr<al::VariantValue> value;
};

struct FileDependency {
  FileDependency(std::string _file = std::string()) { file.filename = _file; }
  FileDependency(DistributedPath file, std::string modified, uint64_t size,
                 std::string hash)
      : file(file), modified(modified), size(size), hash(hash) {}

  FileDependency &operator=(const FileDependency &other);

  DistributedPath file;
  std::string modified;
  uint64_t size = 0;
  std::string hash; // Currently storing CRC32 as a string. Leave possibility to
                    // change hash algorithm in the future
private:
  static void copyValueFromSource(FileDependency &dst,
                                  const FileDependency &src);
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

  static void copyValueFromSource(SourceInfo &dst, const SourceInfo &src);

  // Write copy/move constructor. Write copy operator
  std::string type;
  std::string
      tincId; // TODO add heuristics to match source even if id has changed.
  std::string commandLineArguments;
  DistributedPath workingPath{""};
  std::vector<SourceArgument> arguments;
  std::vector<SourceArgument> dependencies;
  std::vector<FileDependency> fileDependencies;
};

struct CacheEntry {
  std::string timestampStart;
  std::string timestampEnd;
  std::vector<FileDependency> files; // Files cached in this entry
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
                                     bool validateFile = true);
  /**
   * @brief Clear all cached files, and cache information.
   *
   * Although this function will only delete files listed in the cache file, it
   * should be used with caution.
   */
  // TODO we should add support for more granular cache cleanup, for example
  // cache generated by a particular user or a particular node, delete all
  // cache from clients, delete all cache that deals with particular files
  // or parameters.
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

  /**
   * @brief compute CRC32 for file
   * @param filename
   * @return the CRC
   */
  static int32_t computeCrc32(std::string filename);

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

// TODO temporary get rid of this once we use C++20
template <typename TP> time_t __tinc_to_time_t(TP tp) {
  using namespace std::chrono;
  auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() +
                                                      system_clock::now());
  return system_clock::to_time_t(sctp);
}

} // namespace tinc

#endif // CACHEMANAGER_HPP
