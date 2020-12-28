#ifndef TRAJECTORYRENDER_H
#define TRAJECTORYRENDER_H

#include "al/graphics/al_Mesh.hpp"
#include "al/types/al_Color.hpp"
#include "al/math/al_Vec.hpp"
#include "al/ui/al_Parameter.hpp"

#include "SceneObject.hpp"

using namespace tinc;

class TrajectoryRender : public SceneObject {
public:
  TrajectoryRender(std::string id, std::string filename, std::string path = "",
                   uint16_t size = 2);

  void update(double dt) override;
  void onProcess(al::Graphics &g) override;

  // TODO make parameter?
  al::Parameter trajectoryWidth;
  al::Parameter alpha;

private:
  al::VAOMesh mTrajectoryMesh;
};

#endif // TRAJECTORYRENDER_H
