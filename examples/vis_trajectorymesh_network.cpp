#include "tinc/vis/TrajectoryRender.hpp"
#include "tinc/vis/TriangleRender.hpp"

#include "al/app/al_App.hpp"

using namespace tinc;

/* This example shows usage of TrajectoryRender through the TINC server
 * use a remote client like the python client to write to the
 * trajectory render buffer to update display. Use the example notebook:
 * TINC vis scene to control this application.
 *
 * See trajectorymesh.cpp for basic usage of TrajectoryRender
 */

class MyApp : public al::App {

  TrajectoryRender trajectory{"trajectory", "buffer_file.json"};
  TriangleRender triangles{"triangles", "buffer_file.json"};
  TincServer tserver;

  void onCreate() override {
    nav().set(al::Pose{{0, 0, 4}});
    navControl().disable();

    tserver.start();
    trajectory.registerWithTinc(tserver);
    triangles.registerWithTinc(tserver);
  }

  void onAnimate(double dt) override {
    trajectory.update(dt);
    triangles.update(dt);
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
    triangles.onProcess(g);
  }

  void onExit() override { tserver.stop(); }
};

int main() {

  MyApp app;
  app.start();
  return 0;
}
