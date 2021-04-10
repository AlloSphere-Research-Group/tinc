#ifndef SCENEOBJECT_HPP
#define SCENEOBJECT_HPP

#include "al/graphics/al_Graphics.hpp"
#include "al/scene/al_DynamicScene.hpp"

#include "tinc/DiskBufferNetCDFData.hpp"
#include "tinc/TincServer.hpp"

namespace tinc {

/**
 * @brief The SceneObject class
 *
 * When you make a SceneObject subclass, you must implement the use of the scale
 * parameter.
 */
class SceneObject : public al::PositionedVoice {
public:
  SceneObject(std::string id = "", std::string filename = "",
              std::string path = "", uint16_t size = 2);

  void registerWithTincServer(TincServer &server);

  void applyTransformations(al::Graphics &g) override {
    al::PositionedVoice::applyTransformations(g);
    g.scale(scale.get());
  };

  al::ParameterVec3 scale;

protected:
  DiskBufferNetCDFData mBuffer;
};

} // namespace tinc

#endif // SCENEOBJECT_HPP
