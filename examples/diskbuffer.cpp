
#include "tinc/DiskBuffer.hpp"
#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDF.hpp"
#include "tinc/TincServer.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/math/al_Random.hpp"
#include "al/types/al_Color.hpp"
#include "al/ui/al_ControlGUI.hpp"

// This example shows usage of a DiskBuffer that is exposed to the network. This
// example can be used by itself, but can also be controlled through python.

using namespace tinc;
using json = nlohmann::json;

class MyApp : public al::App {
public:
  TincServer tincServer;
  ImageDiskBuffer imageBuffer{"image", "image.png"};
  DiskBufferJson jsonBuffer{"json", "file.json"};
  DiskBufferNetCDFDouble netcdfBuffer{"nc", "file.nc"};

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
    for (size_t i = 0; i < 9; i++) {
      al::Colori c = al::HSV(al::rnd::uniform(), 1.0, 1.0);
      pix.push_back(c.r);
      pix.push_back(c.g);
      pix.push_back(c.b);
    }

    // update the buffer with the new data
    imageBuffer.writePixels(pix.data(), 3, 3);
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
    jsonBuffer.writeJson(exampleJson);
    reportText.set(std::string("Created " + jsonBuffer.getCurrentFileName()));
  }

  void generateNc() {
    auto ncName = netcdfBuffer.getCurrentFileName();
#define NDIMS 1
    int ncid, x_dimid, varid;
    int dimids[NDIMS];

    int retval;

    // generating example data
    std::vector<double> exampleData;
    exampleData.push_back(al::rnd::normal());
    exampleData.push_back(al::rnd::normal());
    exampleData.push_back(al::rnd::normal());
    exampleData.push_back(al::rnd::normal());

    /* Create the file. The NC_CLOBBER parameter tells netCDF to
     * overwrite this file, if it already exists.*/
    if ((retval = nc_create(ncName.c_str(), NC_NETCDF4 | NC_CLOBBER, &ncid))) {
      return;
    }

    /* Define the dimensions. NetCDF will hand back an ID for each. */
    if ((retval = nc_def_dim(ncid, "double", exampleData.size(), &x_dimid))) {
      return;
    }
    dimids[0] = x_dimid;

    /* Define the variable.*/
    if ((retval = nc_def_var(ncid, "data", NC_DOUBLE, NDIMS, dimids, &varid))) {
      return;
    }

    /* End define mode. This tells netCDF we are done defining
     * metadata. */
    if ((retval = nc_enddef(ncid))) {
      return;
    }

    /* Write the pretend data to the file. Although netCDF supports
     * reading and writing subsets of data, in this case we write all
     * the data in one operation. */
    if ((retval = nc_put_var_double(ncid, varid, exampleData.data()))) {
      return;
    }

    /* Close the file. This frees up any internal netCDF resources
     * associated with the file, and flushes any buffers. */
    if ((retval = nc_close(ncid))) {
      return;
    }

    // update the buffer with the new data
    netcdfBuffer.updateData(ncName);
    reportText.set(std::string("Created " + ncName));
  }

  // Application virtual functions

  void onInit() override {
    // Define GUI
    gui << newImage << imageReport << newJson << jsonReport << newNc << ncReport
        << reportText;
    gui.init();

    // Register trigger callbacks
    newImage.registerChangeCallback([&](bool value) { generateImage(); });
    newJson.registerChangeCallback([&](bool value) { generateJson(); });
    newNc.registerChangeCallback([&](bool value) { generateNc(); });

    // Expose buffers on TINC server
    tincServer << imageBuffer << jsonBuffer << netcdfBuffer;
    tincServer.start();
  }

  void onAnimate(double dt) override {
    // checking for updated data
    if (imageBuffer.newDataAvailable()) {
      imageReport.set(true);

      auto imageData = imageBuffer.get();
      // code to utilize new image data
    }

    if (jsonBuffer.newDataAvailable()) {
      jsonReport.set(true);

      auto jsonData = jsonBuffer.get();
      // code to utilize new json data
    }

    if (netcdfBuffer.newDataAvailable()) {
      ncReport.set(true);

      auto ncData = netcdfBuffer.get();
      // code to utilize the new netcdf data
    }
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
