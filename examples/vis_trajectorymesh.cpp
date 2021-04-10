#include "tinc/vis/TrajectoryRender.hpp"

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace tinc;

class MyApp : public al::App {

  bool onKeyDown(al::Keyboard const &k) override {
    // Key presses trigger generation of new path

    if (relativePositions.get() == 1.0) {
      // Relative mode takes a list of relative x,y,z positions, with the first
      // triplet being the starting point and every triplet after describes the
      // relateve vector to the last position
      std::vector<al::Vec3f> posArray;
      posArray.reserve(k.key());
      for (size_t i = 0; i < k.key(); i++) {

        posArray.push_back(al::Vec3f(al::rnd::uniform(-0.5f, 0.5f),
                                     al::rnd::uniform(-0.5f, 0.5f),
                                     al::rnd::uniform(-0.5f, 0.5f)));
      }
      trajectory.setTrajectory(posArray, std::vector<al::Color>());
    } else {
      // In absolute mode you must pass a pair of coordinates that describe each
      // vector to be drawn
      std::vector<std::pair<al::Vec3f, al::Vec3f>> posArray;
      posArray.reserve(k.key());

      for (size_t i = 0; i < k.key(); i++) {
        auto start = al::Vec3f(al::rnd::uniform(-1.5f, 1.5f),
                               al::rnd::uniform(-1.5f, 1.5f),
                               al::rnd::uniform(-1.5f, 1.5f));
        auto end = start + al::Vec3f(al::rnd::uniform(-0.3f, 0.3f),
                                     al::rnd::uniform(-0.3f, 0.3f),
                                     al::rnd::uniform(-0.3f, 0.3f));
        posArray.push_back({start, end});
      }
      trajectory.setTrajectory(posArray, std::vector<al::Color>());
    }

    return true;
  }

  void onCreate() override {
    // Move back so we can see the scene
    nav().set(al::Pose{{0, 0, 4}});
    // Disable keyboard navigation
    navControl().disable();
    // Register trajectory parameters with GUI
    displayText.set("Press any key to generate display");
    gui << trajectory.alpha << trajectory.trajectoryWidth << trajectory.scale
        << relativePositions << displayText;
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
    {
      g.pushMatrix();
      g.translate(0, 0, -8);
      trajectory.onProcess(g);
      g.popMatrix();
    }
    gui.draw(g);
  }

private:
  TrajectoryRender trajectory{"trajectory"};
  al::ControlGUI gui;
  al::ParameterString displayText{"text"};
  al::ParameterBool relativePositions{"relativePostions"};
};

int main() {

  MyApp app;
  app.start();
  return 0;
}
