#include "tinc/CppProcessor.hpp"
#include "tinc/ParameterSpace.hpp"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Font.hpp"
#include "al/io/al_File.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include <fstream>

using namespace al;
using namespace tinc;

struct MyApp : public App {

  ParameterSpace ps;
  CppProcessor processor;

  ControlGUI gui;

  std::string displayText;

  void defineParameterSpace() {
    auto dimension1 = std::make_shared<tinc::ParameterSpaceDimension>("dim1");
    auto dimension2 = std::make_shared<tinc::ParameterSpaceDimension>("dim2");
    auto inner_param =
        std::make_shared<tinc::ParameterSpaceDimension>("inner_param");

    // Create large parameter space
    for (int i = 0; i < 200; i++) {
      dimension1->push_back(i, "L_" + std::to_string(i));
    }
    dimension1->conform();
    dimension1->type = tinc::ParameterSpaceDimension::MAPPED;

    for (int i = 0; i < 220; i++) {
      dimension2->push_back(i / 220.0);
    }
    dimension2->conform();
    dimension2->type = tinc::ParameterSpaceDimension::INDEX;

    for (int i = 0; i < 230; i++) {
      inner_param->push_back(10 + i);
    }
    inner_param->conform();
    inner_param->type = tinc::ParameterSpaceDimension::INTERNAL;

    ps.registerDimension(dimension1);
    ps.registerDimension(dimension2);
    ps.registerDimension(inner_param);

    ps.generateRelativeRunPath = [&](std::map<std::string, size_t> indeces,
                                     ParameterSpace *ps) {
      return "asyncdata/";
    };
    // Create necessary filesystem directories to be populated by data
    ps.createDataDirectories();

    // Register callback after every process call in a parameter sweep
    ps.onSweepProcess = [&](double progress) {
      std::cout << "Progress: " << progress * 100 << "%" << std::endl;
    };

    // Whenever the parameter space point changes, this function is called
    ps.onValueChange = [&](float /*value*/,
                           ParameterSpaceDimension * /*changedDimension*/,
                           ParameterSpace *ps) {
      std::string name =
          "out_" + ps->getDimension("dim1")->getCurrentId() + " -- " +
          std::to_string(ps->getDimension("dim2")->getCurrentIndex()) + " -- " +
          std::to_string(ps->getDimension("inner_param")->getCurrentValue()) +
          ".txt";
      std::ifstream f(ps->currentRunPath() + name);
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
          "out_" + processor.configuration["dim1"].valueStr + " -- " +
          std::to_string(processor.configuration["dim2"].valueInt64) + " -- " +
          std::to_string(processor.configuration["inner_param"].valueDouble) +
          ".txt";
      processor.setOutputFileNames({name});
      return true;
    };

    // processing function takes longer than one second
    processor.processingFunction = [&]() {
      std::string text =
          std::to_string(processor.configuration["dim2"].valueInt64 +
                         processor.configuration["inner_param"].valueDouble);

      std::ofstream f(processor.getRunningDirectory() +
                      processor.getOutputFileNames()[0]);
      f << text << std::endl;
      f.close();
      al_sleep(1.0);
      return true;
    };
  }

  void onInit() override {
    defineParameterSpace();
    initializeComputation();

    parameterServer() << ps.getDimension("dim1")->parameter()
                      << ps.getDimension("dim2")->parameter()
                      << ps.getDimension("inner_param")->parameter();

    gui << ps.getDimension("dim1")->parameter()
        << ps.getDimension("dim2")->parameter()
        << ps.getDimension("inner_param")->parameter();
    gui.init();

    // Now sweep the parameter space asynchronously
    //    ps.sweepAsync(processor);
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
