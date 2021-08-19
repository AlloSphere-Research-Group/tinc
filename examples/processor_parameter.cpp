#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorGraph.hpp"

using namespace al;
using namespace tinc;

// This example shows how parameters can be connected to
// trigger computation chains

struct MyApp : public App {

  ProcessorCpp process{"Process"};

  ParameterSpaceDimension value{"value", ""};
  ControlGUI gui;

  void onInit() override {

    value.getParameter<Parameter>().min(0.0);
    value.getParameter<Parameter>().max(1.0);
    // Register the parameter with the processor
    process.registerDimension(value);
    // Define processing function. The new value is avaialble through
    // the configuration member and the old value is available from the
    // parameter as the new value has not been applied at this point
    process.processingFunction = [&]() {
      std::cout << "new value: " << process.configuration["value"].get<float>()
                << " previous value " << value.getCurrentValue() << std::endl;
      return true;
    };

    gui << *value.getParameterMeta();
    gui.init();
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    gui.draw(g);
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp().start();

  return 0;
}
