#include "tinc/vis/TrajectoryRender.hpp"

#include "al/io/al_Imgui.hpp"

#include <cassert>

using namespace tinc;

TrajectoryRender::TrajectoryRender(std::string id, std::string filename,
                                   std::string path, uint16_t size)
    : SceneObject(id, filename, path, size),
      trajectoryWidth("width", id, 0.1, 0.0001, 0.5),
      alpha("alpha", id, 0.8, 0.0, 1.0) {
  trajectoryWidth.registerChangeCallback([&](float value) {
    // Force a reload. New value will be used in update()
    mBuffer.doneWriting(mBuffer.get());
  });
  alpha.registerChangeCallback([&](float value) {
    // Force a reload. New value will be in in update()
    mBuffer.doneWriting(mBuffer.get());
  });
  registerParameter(alpha);
  registerParameter(trajectoryWidth);
}

void TrajectoryRender::setTrajectory(std::vector<al::Vec3f> positions,
                                     std::vector<al::Color> colors) {
  //    std::unique_lock<std::mutex> lk(m_writeLock);

  NetCDFData newData;
  if (colors.size() == 0) {
    newData.attributes["dataArrangement"] = VariantValue((int64_t)DATA_POS_REL);
  } else {
    if (colors.size() != positions.size() - 1) {
      newData.attributes["dataArrangement"] =
          VariantValue((int64_t)DATA_POS_REL);
      colors.clear();
      std::cout << __FILE__ << ":" << __LINE__
                << " colors and postions size mismatch. Ignoring colors."
                << std::endl;

    } else {
      newData.attributes["dataArrangement"] =
          VariantValue((int64_t)DATA_POS_REL_RGB);
    }
  }

  newData.setType(NC_FLOAT);
  auto &dataVector = newData.getVector<float>();
  dataVector.resize(positions.size() * 3 + colors.size() * 3);
  auto dataIt = dataVector.begin();
  auto colorIt = colors.begin();
  for (auto &pos : positions) {
    *dataIt = pos.x;
    dataIt++;
    *dataIt = pos.y;
    dataIt++;
    *dataIt = pos.z;
    dataIt++;
    if (colorIt != colors.end()) {
      *dataIt = colorIt->r;
      dataIt++;
      *dataIt = colorIt->g;
      dataIt++;
      *dataIt = colorIt->b;
      dataIt++;
      colorIt++;
    }
  }
  mBuffer.setData(newData);
}

void TrajectoryRender::setTrajectory(
    std::vector<std::pair<al::Vec3f, al::Vec3f>> positions,
    std::vector<al::Color> colors) {

  NetCDFData newData;
  if (colors.size() == 0) {
    newData.attributes["dataArrangement"] = VariantValue((int64_t)DATA_POS_ABS);
  } else {
    if (colors.size() != positions.size() - 1) {
      newData.attributes["dataArrangement"] =
          VariantValue((int64_t)DATA_POS_REL);
      colors.clear();
      std::cout << __FILE__ << ":" << __LINE__
                << " colors and postions size mismatch. Ignoring colors."
                << std::endl;

    } else {
      newData.attributes["dataArrangement"] =
          VariantValue((int64_t)DATA_POS_ABS_RGB);
    }
  }

  newData.setType(NC_FLOAT);
  auto &dataVector = newData.getVector<float>();
  dataVector.resize(positions.size() * 6 + colors.size() * 3);
  auto dataIt = dataVector.begin();
  auto colorIt = colors.begin();
  for (auto &pos : positions) {
    *dataIt = pos.first.x;
    dataIt++;
    *dataIt = pos.first.y;
    dataIt++;
    *dataIt = pos.first.z;
    dataIt++;
    *dataIt = pos.second.x;
    dataIt++;
    *dataIt = pos.second.y;
    dataIt++;
    *dataIt = pos.second.z;
    dataIt++;
    if (colorIt != colors.end()) {
      *dataIt = colorIt->r;
      dataIt++;
      *dataIt = colorIt->g;
      dataIt++;
      *dataIt = colorIt->b;
      dataIt++;
      colorIt++;
    }
  }
  mBuffer.setData(newData);
}

void TrajectoryRender::addMarkerToMesh(al::Vec3f start, al::Vec3f end,
                                       al::Color c) {

  // Assumes the plane's normal is the z-axis
  al::Vec3f orthogonalVec =
      (end - start).cross({0, 0, 1}).normalize(trajectoryWidth);

  al::Vec3f orthogonalVec2 =
      (end - start).cross({0, 1, 0}).normalize(trajectoryWidth);

  unsigned int previousSize = mTrajectoryMesh.vertices().size();

  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(start - orthogonalVec);
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(end - orthogonalVec * 0.2f);
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(start + orthogonalVec);
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(end + orthogonalVec * 0.2f);

  mTrajectoryMesh.index(previousSize);
  mTrajectoryMesh.index(previousSize + 1);
  mTrajectoryMesh.index(previousSize + 2);

  mTrajectoryMesh.index(previousSize + 2);
  mTrajectoryMesh.index(previousSize + 3);
  mTrajectoryMesh.index(previousSize + 1);

  //////////////////////////

  previousSize = mTrajectoryMesh.vertices().size();
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(start - orthogonalVec2);
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(end - orthogonalVec2 * 0.2f);
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(start + orthogonalVec2);
  mTrajectoryMesh.color(c);
  mTrajectoryMesh.vertex(end + orthogonalVec2 * 0.2f);

  mTrajectoryMesh.index(previousSize);
  mTrajectoryMesh.index(previousSize + 1);
  mTrajectoryMesh.index(previousSize + 2);

  mTrajectoryMesh.index(previousSize + 2);
  mTrajectoryMesh.index(previousSize + 3);
  mTrajectoryMesh.index(previousSize + 1);

  start = start + end;
}

void TrajectoryRender::update(double dt) {
  if (mBuffer.newDataAvailable()) {
    mTrajectoryMesh.primitive(al::Mesh::TRIANGLES);
    mTrajectoryMesh.reset();
    auto newData = mBuffer.get();
    auto &dataVector = newData->getVector<float>();
    auto dataIt = dataVector.begin();

    al::Color c;

    if (newData->attributes.find("dataArrangement") !=
        newData->attributes.end()) {
      auto arrangement = newData->attributes["dataArrangement"].valueInt64;

      if (arrangement == DATA_POS_REL || arrangement == DATA_POS_REL_RGB) {
        al::Vec3f previousPoint(0, 0, 0);

        size_t pointCount = dataVector.size() / 3;
        if (arrangement == DATA_POS_REL_RGB) {
          pointCount = dataVector.size() / 6;
        }
        size_t counter = pointCount - 1;
        // TODO validate dataVector size
        while (dataIt != dataVector.end()) {

          if (counter == pointCount - 1) {
            // first pass, use first as starting point.
            al::Vec3f firstPoint;
            firstPoint.x = *dataIt;
            dataIt++;
            firstPoint.y = *dataIt;
            dataIt++;
            firstPoint.z = *dataIt;
            dataIt++;
            previousPoint.set(firstPoint);
            counter--;
            continue;
          }
          if (arrangement == DATA_POS_REL) {
            al::HSV hsvColor(0.5 * float(counter) / pointCount, 1.0, 1.0);
            ImGui::ColorConvertHSVtoRGB(hsvColor.h, hsvColor.s, hsvColor.v, c.r,
                                        c.g, c.b);
          } else if (arrangement == DATA_POS_REL_RGB) {
            c.r = *dataIt;
            dataIt++;
            c.g = *dataIt;
            dataIt++;
            c.b = *dataIt;
            dataIt++;
          }
          c.a = alpha;

          al::Vec3f thisMovement;
          thisMovement.x = *dataIt;
          dataIt++;
          thisMovement.y = *dataIt;
          dataIt++;
          thisMovement.z = *dataIt;
          dataIt++;

          al::Vec3f thisPoint = previousPoint + thisMovement;
          addMarkerToMesh(previousPoint, thisPoint, c);
          previousPoint = thisPoint;

          counter--;
        }
      } else if (arrangement == DATA_POS_ABS ||
                 arrangement == DATA_POS_ABS_RGB) {
        size_t pointCount = dataVector.size() / 6;
        if (arrangement == DATA_POS_REL_RGB) {
          pointCount = dataVector.size() / 9;
        }
        size_t counter = pointCount - 1;
        // TODO validate dataVector size
        while (dataIt != dataVector.end()) {
          al::Vec3f startPoint;
          startPoint.x = *dataIt;
          dataIt++;
          startPoint.y = *dataIt;
          dataIt++;
          startPoint.z = *dataIt;
          dataIt++;

          al::Vec3f endPoint;
          endPoint.x = *dataIt;
          dataIt++;
          endPoint.y = *dataIt;
          dataIt++;
          endPoint.z = *dataIt;
          dataIt++;

          if (arrangement == DATA_POS_ABS) {
            al::HSV hsvColor(0.5f * float(counter) / pointCount, 1.0, 1.0);
            ImGui::ColorConvertHSVtoRGB(hsvColor.h, hsvColor.s, hsvColor.v, c.r,
                                        c.g, c.b);
          } else if (arrangement == DATA_POS_ABS_RGB) {
            c.r = *dataIt;
            dataIt++;
            c.g = *dataIt;
            dataIt++;
            c.b = *dataIt;
            dataIt++;
          }
          c.a = alpha;
          addMarkerToMesh(startPoint, endPoint, c);
          counter--;
        }
      } else {
        std::cerr << "Unsupported arrangement type" << std::endl;
      }
      mTrajectoryMesh.update();
    }
  }
}

void TrajectoryRender::onProcess(al::Graphics &g) {
  g.pushMatrix();
  applyTransformations(g);
  g.draw(mTrajectoryMesh);
  g.popMatrix();
}
