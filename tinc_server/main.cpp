
#include "al/app/al_App.hpp"
#include "al/ui/al_ParameterGUI.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/vis/GUI.hpp"

#include "nlohmann/json.hpp"

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

/**
JSON config file should look like:

{
  "rootPathMap" : {
    "": [
          {"serverPath" : "C:/Users/Andres/source/repos",
           "clientPath" : "/shared"
          }
      ]
    }
}
 */
int main(int argc, char *argv[]) {
  TincServerApp app;

  std::string configFile = "tinc_server_config.json";
  if (argc == 2) {
    configFile = argv[1];
  }

  std::ifstream ifs(configFile);
  if (ifs.good()) {
    nlohmann::json j = nlohmann::json::parse(ifs);
    std::cout << "Using config file: " << configFile << std::endl;
    if (j.find("rootPathMap") != j.end() && j["rootPathMap"].is_object()) {
      for (auto &hostMap : j["rootPathMap"].items()) {
        for (auto mapping : hostMap.value()) {
          if (mapping.is_object() && mapping.contains("serverPath") &&
              mapping.contains("clientPath") &&
              mapping["serverPath"].is_string() &&
              mapping["clientPath"].is_string()) {

            app.tserver.setRootMapEntry(mapping["serverPath"],
                                        mapping["clientPath"], hostMap.key());
          } else {
            std::cerr << "ERROR parsing rootMapPath" << std::endl;
          }
        }
      }
    }
  } else {
    std::cout << "JSON config file not found." << std::endl;
  }

  app.start();
  return 0;
}
