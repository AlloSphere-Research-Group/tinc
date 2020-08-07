
#include "tinc/DiskBuffer.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"
#include "tinc/TincServer.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/math/al_Random.hpp"
#include "al/types/al_Color.hpp"
#include "al/ui/al_ControlGUI.hpp"

// This example shows usage of a DiskBuffer that is exposed to the network. This
// example can be used by itself, but can also be controlled through python.

using namespace tinc;

class MyApp : public al::App {
public:
  TincServer tincServer;
  ImageDiskBuffer imageBuffer{"image", "image.png"};
  JsonDiskBuffer jsonBuffer{"json", "file.json"};
  NetCDFDiskBufferDouble netcdfBuffer{"nc", "file.nc"};

  al::Trigger newImage{"newImage"};
  al::Trigger newJson{"newJson"};
  al::Trigger newNc{"newNc"};
  al::ControlGUI gui;

  al::ParameterString reportText{"text"};

  // Functions to generate random data into buffers
  void generateImage() {
    auto imageName = imageBuffer.getCurrentFileName();
    std::vector<unsigned char> pix;
    for (size_t i = 0; i < 9; i++) {
      al::Colori c = al::HSV(al::rnd::uniform(), 1.0, 1.0);
      pix.push_back(c.r);
      pix.push_back(c.g);
      pix.push_back(c.b);
    }

    al::Image::saveImage(imageName, pix.data(), 3, 3);
    imageBuffer.updateData(imageName);
    reportText.set(std::string("Set image " + imageName));
  }

  void generateJson() {
    // TODO finish example
  }

  void generateNc() {
    // TODO finish example
  }

  // Application virtual functions

  void onInit() override {
    // Define GUI
    gui << newImage << newJson << newNc << reportText;
    gui.init();

    // Register trigger callbacks
    newImage.registerChangeCallback([&](bool value) { generateImage(); });
    newJson.registerChangeCallback([&](bool value) { generateJson(); });
    newNc.registerChangeCallback([&](bool value) { generateNc(); });

    // Connect TINC server to network
    tincServer.exposeToNetwork(parameterServer());
    // Expose buffers on TINC server
    tincServer << imageBuffer << jsonBuffer << netcdfBuffer;
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0);
    gui.draw(g);
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp().start();
  return 0;
}
