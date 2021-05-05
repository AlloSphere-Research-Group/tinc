#include "tinc/ParameterSpace.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/vis/GUI.hpp"

#include "al/app/al_App.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#include <fstream>

class MyApp : public al::App {
public:
  tinc::ParameterSpace ps{"PS"};

  // Use a ParameterString to display text
  al::ParameterString text{"text"};

  void onCreate() override {
    // Create dimensions
    auto dimension1 =
        ps.newDimension("dim1", tinc::ParameterSpaceDimension::VALUE);
    auto dimension2 =
        ps.newDimension("dim2", tinc::ParameterSpaceDimension::VALUE);

    // Set possible values for dimensions
    dimension1->setSpaceValues<float>({0.1, 0.1, 0.2, 0.2, 0.3, 0.3});
    dimension1->setSpaceIds({"A", "B", "C", "D", "E", "F"});
    dimension1->conformSpace();

    dimension2->setSpaceValues<float>({10.1, 10.2, 10.1, 10.2, 10.1, 10.2});
    dimension2->setSpaceIds({"A", "B", "C", "D", "E", "F"});
    dimension2->conformSpace();

    ps.onValueChange = [&](tinc::ParameterSpaceDimension *dim,
                           tinc::ParameterSpace *ps) {
      auto id = ps->getCommonId({"dim1", "dim2"});
      text.set(id);
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
