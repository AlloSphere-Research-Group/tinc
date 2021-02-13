#include "gtest/gtest.h"

#include "tinc/ProcessorScript.hpp"
#include "tinc/ProcessorCpp.hpp"
#include "tinc/ProcessorGraph.hpp"

#include "al/math/al_Random.hpp"

#include <fstream>

using namespace tinc;

TEST(Processor, BasicCpp) {
  float value = 0.0;
  ProcessorCpp proc1("proc1");
  proc1.processingFunction = [&]() {
    value = 1.0;
    return true;
  };

  EXPECT_TRUE(proc1.process());
  EXPECT_EQ(value, 1.0);
}

TEST(Processor, BasicScript) {
  float value = 0.0;
  ProcessorScript proc1("proc1");
  proc1.setCommand("ls");

  EXPECT_TRUE(proc1.process());
  EXPECT_EQ(value, 1.0);
}

TEST(Processor, PrepareFunctionCpp) {
  float value = 0.0;
  ProcessorCpp proc1("proc1");
  proc1.processingFunction = [&]() {
    value += 1.0;
    return true;
  };
  proc1.prepareFunction = [&]() {
    value = 1.0;
    return true;
  };

  EXPECT_TRUE(proc1.process());
  EXPECT_EQ(value, 2.0);

  proc1.prepareFunction = [&]() { return false; };

  EXPECT_FALSE(proc1.process());
  EXPECT_EQ(value, 2.0);
}

TEST(Processor, CallbacksCpp) {
  float value = 0.0;
  float before = 0.0;
  float done = 0.0;
  ProcessorCpp proc1("proc1");
  proc1.processingFunction = [&]() {
    value = 1.0;
    return true;
  };
  proc1.registerStartCallback([&]() { before = 2.0; });
  proc1.registerDoneCallback([&](bool) { done = 3.0; });

  EXPECT_TRUE(proc1.process());
  EXPECT_EQ(value, 1.0);
  EXPECT_EQ(before, 2.0);
  EXPECT_EQ(done, 3.0);
}

TEST(Processor, TrigerringCpp) {

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
