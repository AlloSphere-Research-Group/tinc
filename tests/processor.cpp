#include "gtest/gtest.h"

#include "tinc/ProcessorScript.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorGraph.hpp"

#include "al/math/al_Random.hpp"

#include <fstream>

using namespace tinc;

TEST(Processor, Basic) {}

TEST(Processor, Trigerring) {

  ProcessorCpp proc1("proc1");
  ProcessorCpp proc2("proc2");

  proc2.processingFunction = [&]() { return true; };
}

TEST(Processor, ConnectOutputFiles) {

  ProcessorCpp proc1("proc1");
  ProcessorCpp proc2("proc2");

  ProcessorGraph graph("graph");

  graph << proc1 << proc2;

  float value = 1.0;

  proc1.processingFunction = [&]() {
    std::string randName = std::to_string(al::rnd::uniform()) + "txt";
    std::ofstream f(randName);
    f << "hello";
    f.close();
    proc1.setOutputFileNames({randName});
    value = 0.0;
    return true;
  };

  proc2.processingFunction = [&]() {
    //    auto fname = proc2.getInputFileNames()[0];
    //    std::ifstream f(fname);
    //    std::string s;
    //    f >> s;
    //    EXPECT_EQ(s, "hello");
    value = 2.0;
    return true;
  };

  graph.process(true);

  EXPECT_FLOAT_EQ(value, 2.0);
}
