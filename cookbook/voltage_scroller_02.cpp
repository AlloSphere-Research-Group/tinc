#include "tinc/ProcessorCpp.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/ProcessorScript.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_Texture.hpp"
#include <fstream>

using namespace al;
using namespace tinc;

/*
 * This file shows adding interactive on the fly computation to a parameter
 * space. Additionally after a sample is computed, the plot will be loaded and
 * displayed.
 *
 * We need to define what should happen when the current value of the parameter
 * space changes (ParameterSpace::registerChangeCallback). In this case when the
 * parameter space value changes, the python script is called to generate the
 * plot, and the result is then loaded into the open GL texture that is
 * displayed.
 *
 * In this case, the python script has been written to work on the data
 * contained in each directory, for this reason, on the parameter space change
 * callback there is the line:
 * processor.setRunningDirectory(ps.currentRunPath()); That sets the path to be
 * the current one. This is not necessary for the sweep as the sweep function
 * will do this internally. The function sweepAsync is used instead of sweep()
 * so that the GUI remains responsive even while the sweep is being completed.
 *
 * The GUI sliders are generated through the convenience class al::ControlGUI,
 * that will display a GUI control for any parameter that is registered with it.
 *
 *
 */

struct MyApp : public App {
  ParameterSpace ps;
  ProcessorScript processor;
  ControlGUI gui;

  std::string displayText;
  Texture graphTex;

  std::string clean_double_to_string(double value) {
    std::string val_as_string = std::to_string(value);
    auto trimmed = std::unique(val_as_string.begin(), val_as_string.end());
    std::string single_zero(val_as_string.begin(), trimmed);
    if (single_zero.back() == '0') {
      single_zero.pop_back();
    }
    if (single_zero.back() == '.') {
      single_zero.push_back('0');
    }
    return single_zero;
  }

  void defineParameterSpace() {
    auto eci1_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci1");
    auto eci2_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci2");
    auto eci3_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci3");
    auto eci4_dim = std::make_shared<tinc::ParameterSpaceDimension>("eci4");

    // Create large parameter space
    std::vector<float> eci1_values = {-0.25, -0.125, 0.0, 0.125, 0.25};
    std::vector<float> eci2_values = {0.0, 0.5, 1.0, 1.5, 2.5};
    std::vector<float> eci3_values = {0.0, 0.5, 1.0, 1.5};
    std::vector<float> eci4_values = {0.0, 0.25, 0.5};

    std::vector<std::string> eciIds;
    eci1_dim->setSpaceValues(eci1_values);
    for (const auto &val : eci1_values) {
      eciIds.push_back("_" + clean_double_to_string(val));
    }
    eci1_dim->setSpaceIds(eciIds);

    eci1_dim->conformSpace();
    eci1_dim->setSpaceRepresentationType(ParameterSpaceDimension::ID);

    eciIds.clear();
    eci2_dim->setSpaceValues(eci2_values);
    for (const auto &val : eci2_values) {
      eciIds.push_back("_" + clean_double_to_string(val));
    }
    eci2_dim->setSpaceIds(eciIds);
    eci2_dim->setSpaceRepresentationType(ParameterSpaceDimension::ID);
    eci2_dim->conformSpace();

    eciIds.clear();
    eci3_dim->setSpaceValues(eci3_values);
    for (const auto &val : eci3_values) {
      eciIds.push_back("_" + clean_double_to_string(val));
    }
    eci3_dim->setSpaceIds(eciIds);
    eci3_dim->setSpaceRepresentationType(ParameterSpaceDimension::ID);
    eci3_dim->conformSpace();

    eciIds.clear();
    eci4_dim->setSpaceValues(eci4_values);
    for (const auto &val : eci4_values) {
      eciIds.push_back("_" + clean_double_to_string(val));
    }
    eci4_dim->setSpaceIds(eciIds);
    eci4_dim->setSpaceRepresentationType(ParameterSpaceDimension::ID);
    eci4_dim->conformSpace();
    ps.registerDimension(eci1_dim);
    ps.registerDimension(eci2_dim);
    ps.registerDimension(eci3_dim);
    ps.registerDimension(eci4_dim);

    // Determine running path from template that is filled according to current
    // values. Because the dimensions have been set to
    // ParameterSpaceDimension::ID, the current ids will be inserted
    ps.setCurrentPathTemplate(
        "AMX2_spinel_diffusion_0.0_0.0_%%ec1%%_%%eci2%%_%%eci3%%_%%eci4%%");

    // Create necessary filesystem directories to be populated by data
    ps.createDataDirectories();

    // Register callback after every process call in a parameter sweep
    ps.onSweepProcess = [&](double progress) {
      std::cout << "Progress: " << progress * 100 << "%" << std::endl;
    };

    // Whenever the parameter space point changes, this function is called
    ps.onValueChange = [&](ParameterSpaceDimension * /*changedDimension*/,
                           ParameterSpace *ps) {
      processor.setRunningDirectory(ps->currentRunPath());
      processor.process();
      Image img(ps->currentRunPath() + processor.getOutputFileNames()[0]);
      graphTex.resize(img.width(), img.height());
      graphTex.submit(img.array().data(), GL_RGBA, GL_UNSIGNED_BYTE);

      graphTex.filter(Texture::LINEAR);

      return true;
    };
  }

  void initializeComputation() {
    // set local filename here relative to running directory and output
    // directory
    processor.setCommand("python");
    processor.setScriptName("../../plot_voltage.py");
    processor.setOutputFileNames({"voltage_curve.png"});
    // need to assign processor.registerDoneCallback to load image
    //(should be able to use running/output directory to find file)
  }

  void onInit() override {
    defineParameterSpace();
    initializeComputation();

    // GUI sliders
    gui << ps.getDimension("eci1")->parameterMeta();
    gui << ps.getDimension("eci2")->parameterMeta();
    gui << ps.getDimension("eci3")->parameterMeta();
    gui << ps.getDimension("eci4")->parameterMeta();
    gui.init();

    // Now sweep the parameter space asynchronously to fill cache while user is
    // not interacting
    ps.sweepAsync(processor);
  }
  void onCreate() override { graphTex.create2D(512, 512); }

  void onDraw(Graphics &g) override {
    g.clear(0);

    g.pushMatrix();
    g.translate(0, 0, -4);
    g.texture();
    g.quad(graphTex, -1, 1, 2, -1.5);
    g.popMatrix();

    gui.draw(g);
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp().start();

  return 0;
}
