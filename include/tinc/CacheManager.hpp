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

  void clearCache() {
    throw "Mot implemented yet.";
    // TODO implement clear cache
  }

  void appendEntry(CacheEntry &entry) { mEntries.push_back(entry); }

  std::vector<CacheEntry> entries() { return mEntries; };

  std::string cacheDirectory();

  void updateFromDisk();
  void writeToDisk();

  std::string dump();

protected:
  DistributedPath mCachePath;
  std::mutex mCacheLock;

  // In memory cache
  std::vector<CacheEntry> mEntries;

  static void tinc_schema_format_checker(const std::string &format,
                                         const std::string &value);

  nlohmann::json_schema::json_validator mValidator{nullptr,
                                                   tinc_schema_format_checker};
};
}

#endif // CACHEMANAGER_HPP
