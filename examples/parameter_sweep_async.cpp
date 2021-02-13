#include "tinc/ProcessorCpp.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include <fstream>

using namespace al;
using namespace tinc;

// This example shows ParameterSpace::sweepAsync() performing
// a parameter sweep while the GUI remains responsive.

struct MyApp : public App {

  ParameterSpace ps;
  ProcessorCpp processor;

  ControlGUI gui;

  std::string displayText;

  void defineParameterSpace() {
    auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
    auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
    auto inner_param =
        std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

    // Create large parameter space
    std::vector<std::string> ids;
    std::vector<float> values;
    for (int i = 0; i < 200; i++) {
      values.push_back(i);
      ids.push_back("L_" + std::to_string(i));
    }
    dimension1->setSpaceValues(values);
    dimension1->setSpaceIds(ids);
    dimension1->conformSpace();
    dimension1->setSpaceRepresentationType(tinc::ParameterSpaceDimension::ID);

    values.clear();
    for (int i = 0; i < 220; i++) {
      values.push_back(i / 220.0);
    }
    dimension2->setSpaceValues(values);
    dimension2->conformSpace();
    dimension2->setSpaceRepresentationType(
        tinc::ParameterSpaceDimension::INDEX);

    values.clear();
    for (int i = 0; i < 230; i++) {
      values.push_back(10 + i);
    }
    inner_param->setSpaceValues(values);
    inner_param->conformSpace();
    inner_param->setSpaceRepresentationType(
        tinc::ParameterSpaceDimension::VALUE);

    ps.registerDimension(dimension1);
    ps.registerDimension(dimension2);
    ps.registerDimension(inner_param);

    // The running path for the parameter space is fixed
    ps.setCurrentPathTemplate("asyncdata");

    // Create necessary filesystem directories to be populated by data
    ps.createDataDirectories();

    // Register callback after every process call in a parameter sweep
    ps.onSweepProcess = [&](double progress) {
      std::cout << "Progress: " << progress * 100 << "%" << std::endl;
    };

    // Whenever the parameter space point changes, this function is called
    ps.onValueChange = [&](ParameterSpaceDimension * /*changedDimension*/,
                           ParameterSpace *ps) {
      std::string name =
          "out_" + ps->getDimension("dim1")->getCurrentId() + "_" +
          std::to_string(ps->getDimension("dim2")->getCurrentIndex()) + "_" +
          std::to_string(ps->getDimension("inner_param")->getCurrentValue()) +
          ".txt";
      std::ifstream f(ps->currentRelativeRunPath() + name);
      if (f.is_open()) {
        f.seekg(0, std::ios::end);
        displayText.reserve(f.tellg());
        f.seekg(0, std::ios::beg);

        displayText.assign((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
      } else {
        displayText = "Sample not created";
      }
      return true;
    };
  }

  void initializeComputation() {

    processor.prepareFunction = [&]() {
      std::string name =
          "out_" + processor.configuration["dim1"].valueStr + "_" +
          std::to_string(processor.configuration["dim2"].valueInt64) + "_" +
          std::to_string(processor.configuration["inner_param"].valueDouble) +
          ".txt";
      processor.setOutputFileNames({name});
      return true;
    };

    // processing function takes longer than one second
    processor.processingFunction = [&]() {
      std::string text =
          processor.configuration["dim1"].valueStr + " -- " +
          std::to_string(processor.configuration["dim2"].valueInt64) + " -- " +
          std::to_string(processor.configuration["inner_param"].valueDouble);

      std::ofstream f(processor.getOutputDirectory() +
                      processor.getOutputFileNames()[0]);
      f << text << std::endl;
      f.close();
      std::cout << "Wrote "
                << processor.getOutputDirectory() +
                       processor.getOutputFileNames()[0]
                << std::endl;
      al_sleep(0.5);
      return true;
    };

    processor.setVerbose(true);
  }

  void onInit() override {
    defineParameterSpace();
    initializeComputation();

    gui << ps.getDimension("dim1")->getParameterMeta();
    gui << ps.getDimension("dim2")->getParameterMeta();
    gui << ps.getDimension("inner_param")->getParameterMeta();
    gui.init();

    // Now sweep the parameter space asynchronously
    ps.sweepAsync(processor);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    gui.draw(g);
    g.color(0);
    FontRenderer::render(g, displayText.c_str(), {-1, 0, -4}, 0.1);
  }

  void onExit() override {
    ps.stopSweep();
    gui.cleanup();
  }
};

int main() {
  MyApp().start();

  return 0;
}
