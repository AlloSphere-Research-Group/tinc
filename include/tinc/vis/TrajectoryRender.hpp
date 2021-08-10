#ifndef TRAJECTORYRENDER_H
#define TRAJECTORYRENDER_H

#include "al/graphics/al_Mesh.hpp"
#include "al/math/al_Vec.hpp"
#include "al/types/al_Color.hpp"
#include "al/ui/al_Parameter.hpp"

#include "SceneObject.hpp"

namespace tinc {
class TrajectoryRender : public SceneObject {
public:
  TrajectoryRender(std::string id, std::string filename = "trajectory.nc",
                   std::string path = "", std::string rootPath = "",
                   uint16_t size = 2);

  typedef enum {
    DATA_POS_REL = 0,
    DATA_POS_ABS,
    DATA_POS_REL_RGB,
    DATA_POS_ABS_RGB
  } DataArrangement;

  void setTrajectory(std::vector<al::Vec3f> positions,
                     std::vector<al::Color> colors = std::vector<al::Color>());

  void setTrajectory(std::vector<std::pair<al::Vec3f, al::Vec3f>> positions,
                     std::vector<al::Color> colors = std::vector<al::Color>());

  void update(double dt) override;

  void onProcess(al::Graphics &g) override;

  al::Parameter trajectoryWidth;
  al::Parameter alpha;

private:
  al::VAOMesh mTrajectoryMesh;
  void addMarkerToMesh(al::Vec3f start, al::Vec3f end, al::Color c);
};

} // namespace tinc

#endif // TRAJECTORYRENDER_H
