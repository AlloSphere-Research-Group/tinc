#include "tinc/DataPool.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/GUI.hpp"

#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/graphics/al_Texture.hpp"

#include <fstream>

// This example uses all the elements available through the TINC server. You can
// test connection to the TINC server by using the TINC jupyter notebook in the
// tinc-python/jupyter-notebooks directory

// For details on how each individual component works, see the rest of the
// examples

// This app requires that you run the datapool.cpp example prior, as it relies
// on data generated by that example.

struct MyApp : public al::App {
  tinc::ParameterSpace ps;
  tinc::DataPool dp{ps};

  al::Parameter procParameter{"procParam", "", 0.0, -10.0, 10.0};
  tinc::CppProcessor processor;
  float computedValue{0};

  tinc::ImageDiskBuffer dataBuffer{"graph", "output.png"};

  tinc::TincServer tserv;

  // Graphics memebers
  al::Texture graphTex;

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
  }

  void prepareProcessor() {

    processor.processingFunction = [&]() {
      computedValue = 1.0 +
                      ps.getDimension("internalValuesDim")->getCurrentValue() *
                          procParameter.get();

      return true;
    };
    // processor will be run whenever procParameter changes
    processor << procParameter;
  }

  void prepareGui() {
    al::imguiBeginFrame();
    al::ParameterGUI::beginPanel("Parameter Space");
    tinc::gui::drawControls(ps);
    al::ParameterGUI::draw(&procParameter);
    ImGui::Text("Computed value: %f", computedValue);
    al::ParameterGUI::endPanel();
    al::imguiEndFrame();
  }

  // al::App callbacks

  void onCreate() override {
    // Graphics initialization
    al::imguiInit(); // Initialize GUI toolkit

    graphTex.create2D(512, 512);

    // Initialize TINC objects
    prepareParameterSpace();
    prepareProcessor();

    // Configure DataPool
    dp.registerDataFile("datapool_data.json", "internalValuesDim");

    // Configure TincServer
    // Registering the DataPool will bring in the ParameterSpace and
    // ParameterSpaceDimensions associated with it
    tserv << dp;
    // Register the image data buffer
    tserv << dataBuffer;
    tserv.verbose(true); // Show more information

    // Start TINC server with default parameters
    tserv.start();
  }

  void onAnimate(double dt) override {
    prepareGui();

    // Update graphics texture if the buffer has been updated
    if (dataBuffer.newDataAvailable()) {
      if (dataBuffer.get()->array().size() == 0) {
        std::cout << "failed to load image" << std::endl;
      }
      std::cout << "loaded image size: " << dataBuffer.get()->width() << ", "
                << dataBuffer.get()->height() << std::endl;

      graphTex.resize(dataBuffer.get()->width(), dataBuffer.get()->height());
      graphTex.submit(dataBuffer.get()->array().data(), GL_RGBA,
                      GL_UNSIGNED_BYTE);

      graphTex.filter(al::Texture::LINEAR);
    }
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0);
    // Draw the texture
    g.pushMatrix();
    g.translate(0, 0, -4);
    g.quad(graphTex, -1, 1, 2, -1.5);
    g.popMatrix();

    // Draw the GUI control panel
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