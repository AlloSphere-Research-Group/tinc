#include "tinc/DataPool.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/vis/GUI.hpp"

#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

struct MyApp : public al::App {
  tinc::ParameterSpace ps;

  tinc::TincServer tserv;

  void prepareParameterSpace() {
    auto dirDim =
        ps.newDimension("dirDim", tinc::ParameterSpaceDimension::ID,
                        tinc::ParameterSpaceDimension::Datatype::UINT8);
    uint8_t values[] = {0, 2, 4, 6, 8};
    dirDim->appendSpaceValues(values, 5, "datapool_directory_");
    dirDim->conformSpace();

    auto internalValuesDim = ps.newDimension("internalValuesDim");
    float internalValues[] = {-0.3f, -0.2f, -0.1f, 0.0f, 0.1f, 0.2f, 0.3f};
    internalValuesDim->setSpaceValues(internalValues, 7);
    internalValuesDim->conformSpace();

    // The running path for the parameter space is determined by 'dirDim'
    ps.setCurrentPathTemplate("%%dirDim%%");

    internalValuesDim->setCurrentIndex(0);

    // Register parameter space with TincServer
    tserv << ps;
    //    tserv.verbose(true); // Show more information
    tserv.start();
  }

  void prepareGui() {
    al::imguiBeginFrame();
    al::ParameterGUI::beginPanel("Parameter Spaces");
    for (auto *ps : tserv.parameterSpaces()) {
      tinc::vis::drawControls(*ps);
    }
    ImGui::Text("Current path: %s", ps.currentRunPath().c_str());
    tinc::vis::drawTincServerInfo(tserv);
    al::ParameterGUI::endPanel();
    al::imguiEndFrame();
  }

  void onCreate() override {
    al::imguiInit();
    prepareParameterSpace();
  }

  void onAnimate(double dt) override { prepareGui(); }

  void onDraw(al::Graphics &g) override {
    g.clear(0);
    al::imguiDraw();
  }

  void onExit() override {
    tserv.stop();
    al::imguiShutdown();
  }
};

int main() {
  MyApp().start();
  return 0;
}
