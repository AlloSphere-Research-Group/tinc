#include "tinc/vis/TriangleRender.hpp"

#include "al/io/al_Imgui.hpp"

#include <cassert>

using namespace tinc;

TriangleRender::TriangleRender(std::string id, std::string filename,
                               std::string relPath, std::string rootPath,
                               uint16_t size)
    : SceneObject(id, filename, relPath, rootPath, size),
      alpha("alpha", id, 0.8, 0.0, 1.0) {

  mBuffer.setDocumentation(R"(Triangle Render buffer:
 Three triangle vertices followed by RGB
Expects: x0_0,y0_0,z0_0, x0_1,y0_1,z0_1, x0_2,y0_2,z0_2, r0,g0,b0,  ... xn_2,yn_2, zn_2, rn, gn, bn
 )");
  alpha.registerChangeCallback([&](float value) {
    // Force a reload. New value will be in in update()
    mBuffer.doneWriting(mBuffer.get());
  });
  setTriangles(std::vector<al::Vec3f>{}, {});
  registerParameter(alpha);
}

void TriangleRender::setTriangles(std::vector<al::Vec3f> positions,
                                  std::vector<al::Color> colors) {
  //    std::unique_lock<std::mutex> lk(m_writeLock);

  NetCDFData newData;
  if (colors.size() != positions.size() / 3) {
    std::cerr << __FILE__ << ":" << __LINE__ << " Inconsitent data"
              << std::endl;
    return;
  }

  newData.setType(al::VariantType::VARIANT_FLOAT);
  auto &dataVector = newData.getVector<float>();
  dataVector.resize(positions.size() * 3 + colors.size() * 3);
  auto dataIt = dataVector.begin();
  auto colorIt = colors.begin();
  int counter = 0;
  for (auto &pos : positions) {
    counter++;
    *dataIt = pos.x;
    dataIt++;
    *dataIt = pos.y;
    dataIt++;
    *dataIt = pos.z;
    dataIt++;
    if (counter == 3) {
      *dataIt = colorIt->r;
      dataIt++;
      *dataIt = colorIt->g;
      dataIt++;
      *dataIt = colorIt->b;
      dataIt++;
      colorIt++;
      counter = 0;
    }
  }
  mBuffer.setData(newData);
}

void TriangleRender::update(double dt) {
  if (mBuffer.newDataAvailable()) {
    mTrajectoryMesh.primitive(al::Mesh::TRIANGLES);
    mTrajectoryMesh.reset();
    auto newData = mBuffer.get();
    auto &dataVector = newData->getVector<float>();
    auto dataIt = dataVector.begin();

    al::Color c;

    // TODO validate dataVector size
    auto end = dataVector.end();
    while (dataIt != end) {
      for (int i = 0; i < 3; i++) {

        al::Vec3f point;
        point.x = *dataIt;
        dataIt++;
        if (dataIt == end) {
          break;
        }
        point.y = *dataIt;
        dataIt++;
        if (dataIt == end) {
          break;
        }
        point.z = *dataIt;
        dataIt++;
        if (dataIt == end) {
          break;
        }
        mTrajectoryMesh.vertex(point.x, point.y, point.z);
      }

      c.r = *dataIt;
      dataIt++;
      if (dataIt == end) {
        break;
      }
      c.g = *dataIt;
      dataIt++;
      if (dataIt == end) {
        break;
      }
      c.b = *dataIt;
      dataIt++;
      c.a = alpha.get();

      mTrajectoryMesh.color(c);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.color(c);
    }
    mTrajectoryMesh.update();
  }
}

void TriangleRender::onProcess(al::Graphics &g) {
  g.pushMatrix();
  applyTransformations(g);
  g.draw(mTrajectoryMesh);
  g.popMatrix();
}
