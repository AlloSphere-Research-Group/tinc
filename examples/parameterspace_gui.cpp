#include "tinc/CppProcessor.hpp"
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

    dimension1->push_back(0.1, "A");
    dimension1->push_back(0.2, "B");
    dimension1->push_back(0.3, "C");
    dimension1->push_back(0.4, "D");
    dimension1->push_back(0.5, "E");
    dimension1->type = tinc::ParameterSpaceDimension::MAPPED;

    dimension2->push_back(10.1);
    dimension2->push_back(10.2);
    dimension2->push_back(10.3);
    dimension2->push_back(10.4);
    dimension2->push_back(10.5);
    dimension2->type = tinc::ParameterSpaceDimension::INDEX;

    inner_param->push_back(1);
    inner_param->push_back(2);
    inner_param->push_back(3);
    inner_param->push_back(4);
    inner_param->push_back(5);
    inner_param->type = tinc::ParameterSpaceDimension::INTERNAL;
    inner_param->conform();

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
    al::ParameterGUI::beginPanel("Parameter Space");
    tinc::gui::drawControls(ps);
    al::ParameterGUI::endPanel();

    al::imguiEndFrame();

    // Finally, draw the GUI
    al::imguiDraw();
  }
};

int main() {
  MyApp().start();

  return 0;
}
