#include "tinc/vis/SceneObject.hpp"

using namespace tinc;

SceneObject::SceneObject(std::string id, std::string filename, std::string path,
                         uint16_t size)
    : mBuffer(id + "_buffer", filename, path, size) {}

void SceneObject::registerWithTincServer(TincServer &server) {
  server.registerDiskBuffer(mBuffer);
  for (auto *param : mParameters) {
    server.registerParameter(*param);
  }
}
