#include "tinc/DistributedPath.hpp"

#include "al/io/al_File.hpp"

using namespace tinc;

DistributedPath::DistributedPath(std::string filename, std::string relativePath,
                                 std::string rootPath, std::string protocolId)
    : filename(filename), relativePath(relativePath), rootPath(rootPath),
      protocolId(protocolId) {}

std::string DistributedPath::filePath() { return path() + filename; }

std::string DistributedPath::path() {
  return al::File::conformDirectory(rootPath + relativePath);
}
