#include "tinc/DistributedPath.hpp"

#include "al/io/al_File.hpp"

using namespace tinc;

DistributedPath::DistributedPath(std::string filename, std::string relativePath,
                                 std::string rootPath, std::string protocolId)
    : filename(filename), protocolId(protocolId) {
  if (al::File::isRelativePath(filename)) {
    if (relativePath.size() > 0 || rootPath.size() > 0) {
      setPaths(relativePath, rootPath);
    }
  } else {
    if (relativePath.size() > 0 || rootPath.size() > 0) {
      std::cerr << __FILE__ << ":" << __LINE__
                << " ERROR: expecting empty paths, filename is absolute."
                << std::endl;
    }
  }
}

std::string DistributedPath::filePath() { return path() + filename; }

std::string DistributedPath::path() {
  return al::File::conformDirectory(rootPath + relativePath);
}

void DistributedPath::setPaths(std::string relativePath, std::string rootPath) {
  if (relativePath.size() > 0) {
    this->relativePath = al::File::conformDirectory(relativePath);
    if (rootPath.size() > 0) {
      this->rootPath = al::File::conformDirectory(rootPath);
    } else {
      if (!al::File::isRelativePath(this->relativePath)) {
        this->rootPath = "";
      } else {
        this->rootPath = al::File::currentPath();
      }
    }
  } else {
    this->relativePath = "";
    if (rootPath.size() > 0) {
      this->rootPath = al::File::conformDirectory(rootPath);
    } else {
      this->rootPath = al::File::currentPath();
    }
  }
  if (!al::File::exists(this->path())) {
    if (!al::Dir::make(this->path())) {
      std::cerr << "ERROR creating path for diskbuffer: " << this->path()
                << std::endl;
    }
  }
}
