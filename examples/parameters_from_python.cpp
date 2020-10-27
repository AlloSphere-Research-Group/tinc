#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_Imgui.hpp"
//#include "al/ui/al_ParameterGUI.hpp"

#include "tinc/TincServer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/GUI.hpp"

using namespace tinc;

struct TincApp : al::App {
  TincServer tserv;

  void onInit() override {
    tserv.start();
    al::imguiInit();
  }

  void onAnimate(double dt) override {

    auto params = tserv.dimensions();
    al::imguiBeginFrame();
    ImGui::Begin("Remote Controls");
    for (auto *param : params) {

      gui::drawControl(param);
      std::cout << param->values().size() << std::endl;
    }
    ImGui::End();
    al::imguiEndFrame();
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0);
    al::imguiDraw();
  }

  void onExit() override {
    al::imguiShutdown();
    tserv.stop();
  }
};

int main() {
  TincApp().start();
  return 0;
}
