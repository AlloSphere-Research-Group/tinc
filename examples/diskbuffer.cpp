
#include "tinc/DiskBuffer.hpp"
#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDFData.hpp"
#include "tinc/TincServer.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/math/al_Random.hpp"
#include "al/types/al_Color.hpp"
#include "al/ui/al_ControlGUI.hpp"

// This example shows usage of the different DiskBuffer types.
// The buffers are exposed to the network so this
// example can be used by itself, but can also be controlled through python.

using namespace tinc;
using json = nlohmann::json;

class MyApp : public al::App {
public:
  TincServer tincServer;
  DiskBufferImage imageBuffer{"image", "image.png"};
  DiskBufferJson jsonBuffer{"json", "file.json"};
  DiskBufferNetCDFData netcdfBuffer{"nc", "file.nc"};

  al::Trigger newImage{"newImage"};
  al::Trigger newJson{"newJson"};
  al::Trigger newNc{"newNc"};
  al::ControlGUI gui;

  al::ParameterString reportText{"Status"};
  al::ParameterBool imageReport{"Updated"};
  al::ParameterBool jsonReport{"Updated"};
  al::ParameterBool ncReport{"Updated"};

  // Functions to generate random data into buffers
  void generateImage() {

    // generating example image
    std::vector<unsigned char> pix;
    size_t size = 64;
    float hue = al::rnd::uniform() * 0.5;
    float hueIncrement = al::rnd::uniform() * 0.01;
    for (size_t i = 0; i < (size * size); i++) {
      al::Colori c = al::HSV(hue, 1.0, 1.0);
      hue += hueIncrement;
      if (hue > 1.0) {
        hue -= 1.0;
      }
      pix.push_back(c.r);
      pix.push_back(c.g);
      pix.push_back(c.b);
    }

    // update the buffer with the new data
    imageBuffer.writePixels(pix.data(), size, size);
    reportText.set(std::string("Created " + imageBuffer.getCurrentFileName()));
  }

  void generateJson() {

    // generating example json data
    json exampleJson;

    exampleJson["string"] = "example_text";
    exampleJson["bool"] = true;
    exampleJson["int"] = 5;
    exampleJson["double"] = 1.2345;

    // update the buffer with the new data
    jsonBuffer.setData(exampleJson);
    reportText.set(std::string("Created " + jsonBuffer.getCurrentFileName()));
  }

  void generateNc() {

    NetCDFData data;
    // Type must be set before getting the vector.
    data.setType(al::VariantType::VARIANT_DOUBLE);

    auto &exampleData = data.getVector<double>();
    exampleData.push_back(al::rnd::normal());
    exampleData.push_back(al::rnd::normal());
    exampleData.push_back(al::rnd::normal());
    exampleData.push_back(al::rnd::normal());
    netcdfBuffer.setData(data);

    reportText.set(std::string("Created " + netcdfBuffer.getCurrentFileName()));
  }

  // Application virtual functions

  void onInit() override {
    // Define GUI
    gui << newImage << imageReport << newJson << jsonReport << newNc << ncReport
        << reportText;
    gui.init();

    // Register trigger callbacks
    newImage.registerChangeCallback([&](bool /*value*/) { generateImage(); });
    newJson.registerChangeCallback([&](bool /*value*/) { generateJson(); });
    newNc.registerChangeCallback([&](bool /*value*/) { generateNc(); });

    // Allow round robin mode for disk buffers
    imageBuffer.enableRoundRobin(4);
    jsonBuffer.enableRoundRobin(5);
    netcdfBuffer.enableRoundRobin(6);

    // Expose buffers on TINC server
    tincServer << imageBuffer << jsonBuffer << netcdfBuffer;
    tincServer.start();
  }

  void onAnimate(double dt) override {
    // checking for updated data
    if (imageBuffer.newDataAvailable()) {
      imageReport.set(true);

      auto imageData = imageBuffer.get();
      // place here code to utilize new image data
    }

    if (jsonBuffer.newDataAvailable()) {
      jsonReport.set(true);

      auto jsonData = jsonBuffer.get();
      // place here code to utilize new json data
    }

    if (netcdfBuffer.newDataAvailable()) {
      ncReport.set(true);

      auto ncData = netcdfBuffer.get();
      // place here code to utilize the new netcdf data
    }
  }
  void onDraw(al::Graphics &g) override {
    g.clear(0);
    gui.draw(g);
  }

  void onExit() override {
    tincServer.stop(); // Not closing the server leaves a running thread
    gui.cleanup();
  }
};

int main() {
  MyApp().start();
  return 0;
}
