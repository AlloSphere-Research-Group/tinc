#include "tinc/vis/SceneObject.hpp"

using namespace tinc;

SceneObject::SceneObject(std::string id, std::string filename,
                         std::string relPath, std::string rootPath,
                         uint16_t size)
    : scale("scale", id, al::Vec3f(1.0f)),
      mBuffer(id + "_buffer", filename, relPath, rootPath, size) {
  //  setId(id);
  mBuffer.enableRoundRobin(size);
  *this << scale;
}

void SceneObject::registerWithTinc(TincProtocol &tinc) {
  tinc.registerDiskBuffer(mBuffer);
  for (auto *param : parameters()) {
    tinc.registerParameter(*param);
  }
}

void SceneObject::registerUpdateCallback(std::function<void (bool)> func) {
    mBuffer.registerUpdateCallback(func);
}

std::shared_ptr<NetCDFData> SceneObject::getData() { return mBuffer.get(); }
