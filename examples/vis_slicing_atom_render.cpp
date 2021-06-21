#include "tinc/vis/AtomRenderer.hpp"

#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace tinc;

class MyApp : public al::App {

  void onInit() override {
    atoms.init();
    atoms.registerWithTinc(tserver);
    tserver.start();
  }

  void onCreate() override {
    // Move back so we can see the scene
    nav().set(al::Pose{{0, 0, 4}});

    // Box mesh for slicing
    boxMesh.reset();
    boxMesh.primitive(al::Mesh::LINES);

    const float x[2] = {0, 1};
    const float y[2] = {0, 1};
    const float z[2] = {0, 1};

    for (int k = 0; k <= 1; k++) {
      for (int j = 0; j <= 1; j++) {
        for (int i = 0; i <= 1; i++) {
          boxMesh.vertex(x[i], y[j], z[k]);
        }
      }
    }
    static const int I[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 2, 1, 3,
                            4, 6, 5, 7, 0, 4, 1, 5, 2, 6, 3, 7};
    boxMesh.index(I, sizeof(I) / sizeof(*I), 0);

    // Add atoms to start with
    for (int i = 0; i < 2000; i++) {
      atomPositions.push_back(al::Vec3f(al::rnd::uniform(-2.0f, 2.0f),
                                        al::rnd::uniform(-2.0f, 2.0f),
                                        al::rnd::uniform(-2.0f, 2.0f)));
    }
    std::map<std::string, AtomData> atomData;
    atomData["S"] =
        AtomData{(int)atomPositions.size(), "S", 1.0, al::Color(1.0, 1.0, 0)};
    atoms.setPositions(atomPositions, atomData);
    atoms.mSlicingPlaneCorner.set({0, 0, 0});
    atoms.mAtomMarkerSize.set(0.03);

    for (auto *param : atoms.parameters()) {
      gui << *param;
    }
    atoms.mSlicingPlaneNormal.setHint("hide", 0.0);
    gui.init();
  }

  void onAnimate(double dt) override {
    atoms.update(dt);
    rotation += 0.15;
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0.5, 0.5, 0.5, 0.5);
    g.blending(true);
    g.blendTrans();
    g.depthTesting(true);

    {
      g.pushMatrix();
      g.translate(0, 0, -10);
      g.rotate(rotation, 0, 1, 0.5);
      {
        // Draw a box showing (theoretical) boundaries of the data
        g.pushMatrix();
        g.polygonLine();
        g.color(0.8f, 0.8f, 1.0f, 0.9f);
        g.translate(-2, -2, -2);
        g.scale(4);
        g.draw(boxMesh);
        g.popMatrix();
      }
      // Draw atoms
      atoms.onProcess(g);

      // Now draw box showing slicing volume
      {
        g.pushMatrix();
        g.polygonLine();
        g.translate(atoms.mSlicingPlaneCorner.get());
        g.rotate(atoms.mSlicingPlaneQuat);
        g.scale(atoms.mSlicingPlaneSize, atoms.mSlicingPlaneSize,
                atoms.mSlicingPlaneThickness.get());
        g.color(0.8f, 0.8f, 1.0f, 0.9f);
        g.draw(boxMesh);
        g.popMatrix();
      }
      g.popMatrix();
    }
    gui.draw(g);
  }

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

  void onExit() override {
    gui.cleanup();
    tserver.stop();
  }

private:
  SlicingAtomRenderer atoms{"atoms", "atoms.nc"};
  al::ControlGUI gui;
  al::Mesh boxMesh;

  TincServer tserver;

  std::vector<al::Vec3f> atomPositions;
  float rotation = 0;
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
