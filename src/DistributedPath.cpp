#include "tinc/DistributedPath.hpp"

using namespace tinc;

DistributedPath::DistributedPath(std::string filename, std::string relativePath,
                                 std::string rootPath)
    : filename(filename), relativePath(relativePath), rootPath(rootPath) {}
