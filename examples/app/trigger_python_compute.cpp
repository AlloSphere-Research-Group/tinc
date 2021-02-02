#include "al/app/al_DistributedApp.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "tinc/TincServer.hpp"
#include "tinc/DiskBufferJson.hpp"

using namespace al;
using namespace tinc;

struct TincApp : DistributedApp {
  TincServer tserv;

  Parameter range{"range", "", 0.5, "", 0.0, 1.0};
  DiskBufferJson dataBuffer{"random_data", "rand_output.json"};

  ControlGUI gui;

  // Local data that will be filled from disk buffer
  VAOMesh mesh;

  void onInit() override {
    // Expose to network to allow receiving messages to load disk data
    tserv << dataBuffer;

    tserv.start();

    // FIXME expose this parameter through the TINC server instead
    parameterServer() << range;
  }

  void onCreate() override {
    gui.init();
    gui << range;
  }

  void onAnimate(double dt) override {
    if (dataBuffer.newDataAvailable()) {
      std::cout << "New data available. Swapping buffer" << std::endl;
      auto &data = *dataBuffer.get();
      // Make a mesh from the values in the lists
      mesh.reset();
      mesh.primitive(Mesh::LINE_STRIP);
      float x = -1.0;
      if (data.is_array() && data.size() == 10) {
        auto elements = data.get<std::vector<float>>();
        for (auto val : elements) {

          mesh.vertex(x, val, -4.0);
          x += 0.2;
        }
      }

      mesh.update();
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0);

    g.color(1.0);
    g.draw(mesh);
    gui.draw(g);
  }

  void onExit() override {
    gui.cleanup();
    tserv.stop();
  }
};

int main() {
  TincApp().start();
  return 0;
}
