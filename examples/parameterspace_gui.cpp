#include "tinc/ProcessorCpp.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/GUI.hpp"

#include "al/app/al_App.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#include <fstream>

class MyApp : public al::App {

  tinc::ParameterSpace ps;

  void onCreate() override {
    auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
    auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
    auto inner_param =
        std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

    dimension1->setSpaceValues({0.1, 0.2, 0.3, 0.4, 0.5});
    dimension1->setSpaceIds({"A", "B", "C", "D", "E"});
    dimension1->setSpaceRepresentationType(tinc::ParameterSpaceDimension::ID);

    dimension2->setSpaceValues({10.1, 10.2, 10.3, 10.4, 10.5});
    dimension2->setSpaceRepresentationType(
        tinc::ParameterSpaceDimension::INDEX);

    inner_param->setSpaceValues({1, 2, 3, 4, 5});
    inner_param->setSpaceRepresentationType(
        tinc::ParameterSpaceDimension::VALUE);

    inner_param->conformSpace();

    ps.registerDimension(dimension1);
    ps.registerDimension(dimension2);
    ps.registerDimension(inner_param);

    // We must initialize ImGUI ourselves:
    al::imguiInit();
    // Disable mouse nav to avoid naving while changing gui controls.
    navControl().useMouse(false);
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0);

    al::imguiBeginFrame();
    al::ParameterGUI::beginPanel("Parameter Space", 0, 0, 640, 150);
    tinc::gui::drawControls(ps);
    al::ParameterGUI::endPanel();

    al::imguiEndFrame();

    // Finally, draw the GUI
    al::imguiDraw();
  }

  void onExit() override { al::imguiShutdown(); }
};

int main() {
  MyApp().start();

  return 0;
}
