#include "al/app/al_DistributedApp.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

#include "tinc/TincServer.hpp"

using namespace al;
using namespace tinc;

struct TincApp : DistributedApp {
  Parameter value{"value", "", 0.0, "", 0.0, 1.0};
  ParameterString stringParam{"string", "", "default value"};
  ParameterInt intValue{"int"};
  Trigger resetString{"ResetString"};
  Trigger ping{"Ping"};

  ControlGUI gui;

  TincServer tserv;

  void onCreate() {
    gui.init();
    // Register parameters with GUI
    gui << value << stringParam << resetString << intValue << ping;
    parameterServer() << value << stringParam << intValue;
    parameterServer().print();
    parameterServer().verbose();

    // Start TINC server
    tserv.setVerbose(true);
    tserv.start(34450, "localhost");

    // Callbacks for triggers
    resetString.registerChangeCallback(
        [this](bool value) { stringParam.reset(); });

    // The ping button triggers a ping request
    ping.registerChangeCallback([this](bool value) { tserv.ping(); });
  }

  void onAnimate(double dt) {}

  void onDraw(Graphics &g) {
    g.clear(0.2);
    gui.draw(g);
  }

  void onExit() { gui.cleanup(); }
};

int main() {
  TincApp().start();
  return 0;
}
