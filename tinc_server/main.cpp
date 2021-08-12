
#include "al/app/al_App.hpp"
#include "al/ui/al_ParameterGUI.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/vis/GUI.hpp"

using namespace tinc;

class TincServerApp : public al::App {
public:
  void onInit() override { tserver.start(); }

  void onCreate() override { al::imguiInit(); }

  void onAnimate(double dt) override {
    al::imguiBeginFrame();
    al::ParameterGUI::beginPanel("Tinc Server");

    tinc::vis::drawTincServerInfo(tserver);
    for (auto *dim : tserver.dimensions()) {
      tinc::vis::drawControl(dim);
    }

    al::ParameterGUI::endPanel();
    al::imguiEndFrame();
  }

  void onDraw(al::Graphics &g) override {

    g.clear();
    al::imguiDraw();
  }

  void onExit() override {
    al::imguiShutdown();
    tserver.stop();
  }

  TincServer tserver;

private:
};

int main() {
  TincServerApp app;

  app.tserver.setRootMapEntry("C:/Users/Andres/source/repos", "/shared");

  app.start();
  return 0;
}
