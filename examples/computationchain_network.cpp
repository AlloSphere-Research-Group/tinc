#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "tinc/ComputationChain.hpp"
#include "tinc/CppProcessor.hpp"
#include "tinc/TincServer.hpp"

using namespace al;
using namespace tinc;

// This example shows how to expose computation chains on the network.
// Use a python notebook to connect to this app

struct MyApp : public App {

  ComputationChain mainChain{"main"};
  ComputationChain joinChain{ComputationChain::PROCESS_ASYNC};
  ComputationChain chain1_3;
  CppProcessor process1{"1"};
  CppProcessor process2{"2"};
  CppProcessor process3{"3"};
  CppProcessor process4{"4"};

  TincServer tserver;

  Parameter value{"value", "", 0.0, "", 0.0, 1.0};
  ParameterString displayResult{"displayResult"};
  ControlGUI gui;

  double data1;
  double data2;

  void onInit() override {

    // Define processing functions
    process1.processingFunction = [&]() {
      data1 = mainChain.configuration["value"].valueDouble + 1.0;
      return true;
    };

    process2.processingFunction = [&]() {
      data2 = -1.0 * mainChain.configuration["value"].valueDouble;
      return true;
    };

    process3.processingFunction = [&]() {
      data1 += 1.0;
      return true;
    };

    // Organize processing chains
    chain1_3 << process1 << process3;
    joinChain << chain1_3 << process2;
    mainChain << joinChain;

    // Register a parameter that triggers computation
    mainChain << value;

    // Expose the computation chain on the TINC server
    tserver << mainChain;

    // Start TINC server
    tserver.verbose(true);
    tserver.start(34450, "localhost");

    // Whenever the chain processes, this function will print out the current
    // values.
    mainChain.registerDoneCallback([&](bool ok) {
      std::cout << (ok ? "OK " : "NOTOK ") << " data1=" << data1
                << " data2=" << data2 << std::endl;
      if (ok) {
        displayResult.set(std::to_string(data1) + " : " +
                          std::to_string(data2));
      }
    });

    // Put the value Parameter in the GUI and the parameter used to display
    // result.
    gui << value << displayResult;
    gui.init();
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    gui.draw(g);
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp().start();

  return 0;
}
