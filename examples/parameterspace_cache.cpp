#include "tinc/ProcessorCpp.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/vis/GUI.hpp"

#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#include <fstream>

class MyApp : public al::App {
public:
  tinc::ParameterSpace ps{"PS"};
  tinc::ProcessorCpp processor{"SimpleProcessor"};

  // Use a ParameterString to display text
  al::ParameterString text{"text"};

  void onCreate() override {
    // Create dimensions
    auto dimension1 =
        ps.newDimension("dim1", tinc::ParameterSpaceDimension::ID);
    auto dimension2 =
        ps.newDimension("dim2", tinc::ParameterSpaceDimension::INDEX);
    auto inner_param =
        ps.newDimension("inner_param", tinc::ParameterSpaceDimension::VALUE);

    // Set possible values for dimensions
    dimension1->setSpaceValues<float>({0.1, 0.2, 0.3, 0.4, 0.5});
    dimension1->setSpaceIds({"A", "B", "C", "D", "E"});
    dimension1->conformSpace();

    dimension2->setSpaceValues<float>({10.1, 10.2, 10.3, 10.4, 10.5});
    dimension2->conformSpace();

    inner_param->setSpaceValues<float>({1, 2, 3, 4, 5});
    inner_param->conformSpace();

    // Enable caching
    ps.enableCache("parameterspace_cache_cache");

    // Processor
    processor.setOutputFileNames({"proc_out.txt"});
    processor.processingFunction = [&]() {
      std::ofstream f(processor.getOutputFileNames()[0]);
      if (f.good()) {
        // Use resolveFilename to create a string from parameter values
        // just for demonstration purposes.
        f << ps.resolveFilename("%%dim1%%_%%dim2%%_%%inner_param%%");
        f.close();
      }
      al::al_sleep(5); // Sleep 5 seconds to simulate long processing
      return true;
    };

    // Run processor every time a value changes.
    ps.onValueChange = [&](tinc::ParameterSpaceDimension * /*changedDimension*/,
                           tinc::ParameterSpace *ps) {
      ps->runProcess(processor);
      std::ifstream f(processor.getOutputFileNames()[0]);
      if (f.good()) {
        std::stringstream ss;
        ss << f.rdbuf();
        text.set(ss.str());
        f.close();
      }
    };

    // GUI
    al::imguiInit();
  }

  void onAnimate(double /*dt*/) override {
    al::imguiBeginFrame();
    al::ParameterGUI::beginPanel("Parameter Space", 0, 0, 640, 150);
    tinc::vis::drawControls(ps);
    al::ParameterGUI::draw(&text);
    al::ParameterGUI::endPanel();

    al::imguiEndFrame();
  }

  void onDraw(al::Graphics &g) override {
    g.clear();
    // Finally, draw the GUI
    al::imguiDraw();
  }

  void onExit() override { al::imguiShutdown(); }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
