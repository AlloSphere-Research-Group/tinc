#include "tinc/vis/TrajectoryRender.hpp"

#include "al/io/al_Imgui.hpp"

#include <cassert>

TrajectoryRender::TrajectoryRender(std::string id, std::string filename,
                                   std::string path, uint16_t size)
    : SceneObject(id, filename, path, size) {}

void TrajectoryRender::update(double dt) {
  if (buffer.newDataAvailable()) {
    mTrajectoryMesh.primitive(al::Mesh::TRIANGLES);
    mTrajectoryMesh.reset();
    auto newData = buffer.get();
    size_t counter = newData->size() - 1;

    al::Vec3f previousPoint(0, 0, 0);
    al::Vec3f thisPoint(0, 0, 0);
    size_t pointCount = newData->size();
    al::Color c;

    for (auto point : *newData) {

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
          c.a = alpha;
        } else if (point[0].size() == 4) {
          auto pointVector = point.at(0).get<std::vector<float>>();
          thisPoint.set(pointVector.data());
          auto newColor = point.at(0).get<std::vector<float>>();
          c.set(newColor.data());
        } else {
          std::cerr
              << __FUNCTION__
              << "Unexpected data shape for TrajectoryRender JsonDiskBuffer"
              << std::endl;
        }
      } else if (point.size() == 2) {
        // Full vector description (start and end)
        if (point[0].size() == 3) {
          auto previousPointVector = point.at(0).get<std::vector<float>>();
          previousPoint.set(previousPointVector.data());
          auto thisPointVector = point.at(1).get<std::vector<float>>();
          thisPoint.set(thisPointVector.data());
          al::HSV hsvColor(0.5f * float(counter) / pointCount, 1.0, 1.0);
          ImGui::ColorConvertHSVtoRGB(hsvColor.h, hsvColor.s, hsvColor.v, c.r,
                                      c.g, c.b);
          c.a = alpha;
        } /*else if (point[0].size() == 4) {
          auto previousPointVector = point.at(0).get<std::vector<float>>();
          previousPoint.set(previousPointVector.data());
          auto thisPointVector = point.at(1).get<std::vector<float>>();
          thisPoint.set(thisPointVector.data());
          ;
          auto newColor = point.at(0).get<std::vector<float>>();
          c.set(newColor.data());
        } */ else {
          std::cerr
              << __FUNCTION__
              << "Unexpected data shape for TrajectoryRender JsonDiskBuffer"
              << std::endl;
        }
      } else {
        std::cerr << __FUNCTION__
                  << "Unexpected data shape for TrajectoryRender JsonDiskBuffer"
                  << std::endl;
      }

      // Assumes the plane's normal is the z-axis
      al::Vec3f thisMovement = thisPoint - previousPoint;
      al::Vec3f orthogonalVec =
          thisMovement.cross({0, 0, 1}).normalize(trajectoryWidth);

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
      previousPoint = previousPoint + thisMovement;
      counter--;
    }
    mTrajectoryMesh.update();
  }
}

void TrajectoryRender::onProcess(al::Graphics &g) { g.draw(mTrajectoryMesh); }
