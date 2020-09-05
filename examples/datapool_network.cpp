#include "tinc/DataPool.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/GUI.hpp"

#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

//#include "nlohmann/json.hpp"
// using json = nlohmann::json;

#include <fstream>

// This app requires that you run the datapool.cpp example prior, as it relies
// on data generated by that example.

struct MyApp : public al::App {
  tinc::ParameterSpace ps;
  tinc::DataPool dp{ps};

  tinc::TincServer tserv;

  void prepareParameterSpace() {
    auto dirDim =
        ps.newDimension("dirDim", tinc::ParameterSpaceDimension::MAPPED);
    uint8_t values[] = {0, 2, 4, 6, 8};
    dirDim->append(values, 5, "datapool_directory_");
    dirDim->conform();

    auto internalValuesDim = ps.newDimension("internalValuesDim");
    float internalValues[] = {-0.3f, -0.2f, -0.1f, 0.0f, 0.1f, 0.2f, 0.3f};
    internalValuesDim->append(internalValues, 7);
    internalValuesDim->conform();

    ps.generateRelativeRunPath = [](std::map<std::string, size_t> indeces,
                                    tinc::ParameterSpace *ps) {
      std::string path = ps->getDimension("dirDim")->idAt(indeces["dirDim"]);
      return path;
    };
    internalValuesDim->setCurrentIndex(0);

    // Now we will access the data
    dp.registerDataFile("datapool_data.json", "internalValuesDim");

    // Configure TincServer
    tserv << dp;
    //    tserv.verbose(true); // Show more information
    tserv.start();
  }

  void prepareGui() {
    al::imguiBeginFrame();
    al::ParameterGUI::beginPanel("Parameter Space");
    tinc::gui::drawControls(ps);
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
