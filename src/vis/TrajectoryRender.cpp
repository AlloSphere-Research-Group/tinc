#include "tinc/vis/TrajectoryRender.hpp"

#include "al/io/al_Imgui.hpp"

#include <cassert>

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
    // Force a reload
    // Force a reload. New value will be in in update()
    mBuffer.doneWriting(mBuffer.get());
  });
  mParameters.push_back(&alpha);
  mParameters.push_back(&trajectoryWidth);
}

void TrajectoryRender::update(double dt) {
  if (mBuffer.newDataAvailable()) {
    mTrajectoryMesh.primitive(al::Mesh::TRIANGLES);
    mTrajectoryMesh.reset();
    auto newData = mBuffer.get();
    size_t counter = newData->size() - 1;

    al::Vec3f previousPoint(0, 0, 0);
    al::Vec3f thisPoint(0, 0, 0);
    size_t pointCount = newData->size();
    al::Color c;

    for (auto point : *newData) {
      al::Vec3f thisMovement;
      if (point.size() == 1) {
        // Relative position only. Need to store previous and use automatic
        // colors.
        if (counter == pointCount - 1) {
          // first pass, use first as starting point.
          if (point.size() == 1 && point[0].size() == 3) {
            auto firstPoint = point.at(0).get<std::vector<float>>();
            previousPoint.set(firstPoint.data());
          }
          counter--;
          continue;
        }
        if (point[0].size() == 3) {
          auto pointVector = point.at(0).get<std::vector<float>>();
          thisPoint.set(pointVector.data());
          al::HSV hsvColor(0.5f * float(counter) / pointCount, 1.0, 1.0);
          ImGui::ColorConvertHSVtoRGB(hsvColor.h, hsvColor.s, hsvColor.v, c.r,
                                      c.g, c.b);
          thisMovement = thisPoint;
          c.a = alpha;
        } /*else if (point[0].size() == 4) {
          auto pointVector = point.at(0).get<std::vector<float>>();
          thisPoint.set(pointVector.data());
          auto newColor = point.at(0).get<std::vector<float>>();
          c.set(newColor.data());
          thisMovement = thisPoint;
        } */ else {
          std::cerr
              << __FUNCTION__
              << "Unexpected data shape for TrajectoryRender DiskBufferJson"
              << std::endl;
        }
      } else if (point.size() == 2 || point.size() == 3) {
        // Full vector description (start and end)
        if (point[0].size() == 3 && point[1].size() == 3) {
          auto previousPointVector = point.at(0).get<std::vector<float>>();
          previousPoint.set(previousPointVector.data());
          auto thisPointVector = point.at(1).get<std::vector<float>>();
          thisPoint.set(thisPointVector.data());
          al::HSV hsvColor(0.5f * float(counter) / pointCount, 1.0, 1.0);
          ImGui::ColorConvertHSVtoRGB(hsvColor.h, hsvColor.s, hsvColor.v, c.r,
                                      c.g, c.b);
          c.a = alpha;
          thisMovement = thisPoint - previousPoint;
        } else {
          std::cerr
              << __FUNCTION__
              << "Unexpected data shape for TrajectoryRender DiskBufferJson"
              << std::endl;
        }
        if (point.size() == 3) {
          auto colorVector = point.at(2).get<std::vector<float>>();
          if (colorVector.size() == 3) {
            c.set(colorVector[0], colorVector[1], colorVector[2]);
            c.a = alpha;
          } else if (colorVector.size() == 4) {
            c.set(colorVector.data());
            c.a = c.a * alpha;
          }
        }
      } else {
        std::cerr << __FUNCTION__
                  << "Unexpected data shape for TrajectoryRender DiskBufferJson"
                  << std::endl;
      }

      // Assumes the plane's normal is the z-axis
      al::Vec3f orthogonalVec =
          thisMovement.cross({0, 0, 1}).normalize(trajectoryWidth);

      al::Vec3f orthogonalVec2 =
          thisMovement.cross({0, 1, 0}).normalize(trajectoryWidth);

      unsigned int previousSize = mTrajectoryMesh.vertices().size();
      // TODO put back undoing wrapping of atom
      //    if (thisMovement.mag() >
      //        fabs(mDataBoundaries.max.x - mDataBoundaries.min.x) /
      //            2.0f) { // Atom is wrapping around
      //      //              c = Color(0.8f, 0.8f, 0.8f, 1.0f);
      //      thisMovement = -thisMovement;
      //      thisMovement.normalize(previousMag);
      //    } else {
      //      previousMag = thisMovement.mag();
      //    }
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint - orthogonalVec);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint + thisMovement -
                             orthogonalVec * 0.2f);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint + orthogonalVec);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint + thisMovement +
                             orthogonalVec * 0.2f);

      mTrajectoryMesh.index(previousSize);
      mTrajectoryMesh.index(previousSize + 1);
      mTrajectoryMesh.index(previousSize + 2);

      mTrajectoryMesh.index(previousSize + 2);
      mTrajectoryMesh.index(previousSize + 3);
      mTrajectoryMesh.index(previousSize + 1);

      //////////////////////////

      previousSize = mTrajectoryMesh.vertices().size();
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint - orthogonalVec2);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint + thisMovement -
                             orthogonalVec2 * 0.2f);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint + orthogonalVec2);
      mTrajectoryMesh.color(c);
      mTrajectoryMesh.vertex(previousPoint + thisMovement +
                             orthogonalVec2 * 0.2f);

      mTrajectoryMesh.index(previousSize);
      mTrajectoryMesh.index(previousSize + 1);
      mTrajectoryMesh.index(previousSize + 2);

      mTrajectoryMesh.index(previousSize + 2);
      mTrajectoryMesh.index(previousSize + 3);
      mTrajectoryMesh.index(previousSize + 1);

      previousPoint = previousPoint + thisMovement;
      counter--;
    }
    mTrajectoryMesh.update();
  }
}

void TrajectoryRender::onProcess(al::Graphics &g) { g.draw(mTrajectoryMesh); }
