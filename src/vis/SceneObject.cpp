#include "tinc/vis/SceneObject.hpp"

using namespace tinc;

SceneObject::SceneObject(std::string id, std::string filename, std::string path,
                         uint16_t size)
    : scale("scale", id, al::Vec3f(1.0f)),
      mBuffer(id + "_buffer", filename, path, size) {
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

std::shared_ptr<NetCDFData> SceneObject::getData() { return mBuffer.get(); }
