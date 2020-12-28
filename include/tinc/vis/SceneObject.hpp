#ifndef SCENEOBJECT_HPP
#define SCENEOBJECT_HPP

#include "al/graphics/al_Graphics.hpp"
#include "al/scene/al_DynamicScene.hpp"

#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/TincServer.hpp"

namespace tinc {

class SceneObject : public al::PositionedVoice {
public:
  SceneObject(std::string id = "", std::string filename = "",
              std::string path = "", uint16_t size = 2);

  void registerWithTincServer(TincServer &server);

  bool writeJson(nlohmann::json &newJsonData) {
    return mBuffer.writeJson(newJsonData);
  }

protected:
  JsonDiskBuffer mBuffer;
  std::vector<al::ParameterMeta *> mParameters;
};

} // namespace tinc

#endif // SCENEOBJECT_HPP
