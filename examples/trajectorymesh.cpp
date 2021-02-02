#include "tinc/vis/TrajectoryRender.hpp"

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace tinc;

class MyApp : public al::App {

  bool onKeyDown(al::Keyboard const &k) override {
    // Key presses trigger generation of new path
    nlohmann::json newJson;
    for (int i = 0; i < k.key(); i++) {
      std::vector<std::vector<float>> posArray;
      std::vector<float> pos;
      for (int j = 0; j < 3; j++) {
        pos.push_back(al::rnd::uniform(-0.5f, 0.5f));
      }
      posArray.push_back(pos);
      newJson.push_back(posArray);
    }
    trajectory.writeJson(newJson);
    return true;
  }

  void onCreate() override {
    // Move back so we can see the scene
    nav().set(al::Pose{{0, 0, 4}});
    // Disable keyboard navigation
    navControl().disable();
    // Register trajectory parameters with GUI
    displayText.set("Press any key to generate display");
    gui << trajectory.alpha << trajectory.trajectoryWidth << displayText;
    gui.init();
  }

  void onAnimate(double dt) override {
    // We must call the update(0 function to make sure we process any new buffer
    // data here. It must be done in the graphics thread (i.e. onAnimate() or
    // onDraw() as this writes the mesh to the GPU, which can only happen there.
    trajectory.update(dt);
  }

  void onDraw(al::Graphics &g) override {
    g.clear();
    // Display the color embedded in the mesh data
    g.meshColor();
    g.blendTrans();
    g.blending(true);
    g.depthTesting(true);
    // Draw the trajectory mesh
    trajectory.onProcess(g);
    gui.draw(g);
  }

private:
  TrajectoryRender trajectory{"trajectory", "buffer_file.json"};
  al::ControlGUI gui;
  al::ParameterString displayText{"text"};
};

int main() {

  MyApp app;
  app.start();
  return 0;
}
