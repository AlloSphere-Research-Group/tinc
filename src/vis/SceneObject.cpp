#include "tinc/vis/SceneObject.hpp"

using namespace tinc;

SceneObject::SceneObject(std::string id, std::string filename, std::string path,
                         uint16_t size)
    : scale("scale", id, al::Vec3f(1.0f)),
      mBuffer(id + "_buffer", filename, path, size) {
  mBuffer.enableRoundRobin(size);
  *this << scale;
}

void SceneObject::registerWithTincServer(TincServer &server) {
  server.registerDiskBuffer(mBuffer);
  for (auto *param : parameters()) {
    server.registerParameter(*param);
  }
}
