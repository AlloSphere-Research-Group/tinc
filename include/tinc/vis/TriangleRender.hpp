#ifndef TRIANGLERENDER_H
#define TRIANGLERENDER_H

#include "al/graphics/al_Mesh.hpp"
#include "al/math/al_Vec.hpp"
#include "al/types/al_Color.hpp"
#include "al/ui/al_Parameter.hpp"

#include "SceneObject.hpp"

namespace tinc {
class TriangleRender : public SceneObject {
public:
  TriangleRender(std::string id, std::string filename = "tetrahedron.nc",
                 std::string path = "", std::string rootPath = "",
                 uint16_t size = 2);

  void setTriangles(std::vector<al::Vec3f> positions,
                    std::vector<al::Color> colors = std::vector<al::Color>());

  void update(double dt) override;

  void onProcess(al::Graphics &g) override;

  al::Parameter alpha;

private:
  al::VAOMesh mTrajectoryMesh;
};

} // namespace tinc

#endif // TRIANGLERENDER_H
