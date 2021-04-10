#include "tinc/vis/AtomRenderer.hpp"

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace tinc;

class MyApp : public al::App {

  bool onKeyDown(al::Keyboard const &k) override {
    if (k.key() == '[') {
      // Remove an atom
      if (atomPositions.size() > 0) {
        atomPositions.resize(atomPositions.size() - 1);
      }
    } else if (k.key() == ']') {
      atomPositions.push_back(al::Vec3f(al::rnd::uniform(-1.5f, 1.5f),
                                        al::rnd::uniform(-1.5f, 1.5f),
                                        al::rnd::uniform(-1.5f, 1.5f)));
    }
    std::map<std::string, AtomData> atomData;
    atomData["S"] =
        AtomData{(int)atomPositions.size(), "S", 1.0, al::Color(1.0, 1.0, 0)};
    atoms.setPositions(atomPositions, atomData);
    return true;
  }

  void onCreate() override {
    // Move back so we can see the scene
    nav().set(al::Pose{{0, 0, 4}});

    // Add 4 atoms to start with
    for (int i = 0; i < 18; i++) {
      atomPositions.push_back(al::Vec3f(al::rnd::uniform(-1.5f, 1.5f),
                                        al::rnd::uniform(-1.5f, 1.5f),
                                        al::rnd::uniform(-1.5f, 1.5f)));
    }
    std::map<std::string, AtomData> atomData;
    atomData["S"] =
        AtomData{(int)atomPositions.size(), "S", 1.0, al::Color(1.0, 1.0, 0)};
    atoms.setPositions(atomPositions, atomData);

    for (auto *param : atoms.parameters()) {
      gui << *param;
    }
    gui.init();
  }

  void onInit() override { atoms.init(); }
  void onAnimate(double dt) override { atoms.update(dt); }

  void onDraw(al::Graphics &g) override {
    g.clear(0.5, 0.5, 0.5);
    g.blendTrans();
    g.blending(true);
    g.depthTesting(true);

    {
      g.pushMatrix();
      g.translate(0, 0, -10);
      atoms.onProcess(g);
      g.popMatrix();
    }
    gui.draw(g);
  }

  void onExit() override { gui.cleanup(); }

private:
  SlicingAtomRenderer atoms{"atoms", "atoms.nc"};
  al::ControlGUI gui;

  std::vector<al::Vec3f> atomPositions;
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
