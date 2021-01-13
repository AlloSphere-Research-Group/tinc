#ifndef CACHEMANAGER_HPP
#define CACHEMANAGER_HPP

#include "nlohmann/json.hpp"
#include "nlohmann/json-schema.hpp"

#include "tinc/DistributedPath.hpp"
#include "tinc/VariantValue.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <cinttypes>

namespace tinc {

struct UserInfo {
  std::string userName;
  std::string userHash;
  std::string ip;
  uint16_t port;
  bool server{false};
};

struct SourceArgument {
  std::string id;
  VariantValue value;
};

struct SourceInfo {
  std::string type;
  std::string
      tincId; // TODO add heuristics to match source even if id has changed.
  std::string commandLineArguments;
  std::string hash;
  std::vector<SourceArgument> arguments;
  std::vector<SourceArgument> dependencies;
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
   * @return vector of CacheEntry objects
   */
  std::vector<CacheEntry> entries() { return mEntries; };

  std::vector<std::string> findCache(const SourceInfo &sourceInfo,
                                     bool verifyHash = true);
  /**
   * @brief Clear all cached files, and cache information.
   */
  void clearCache() {
    throw "Mot implemented yet.";
    // TODO implement clear cache
  }

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
}

#endif // CACHEMANAGER_HPP
