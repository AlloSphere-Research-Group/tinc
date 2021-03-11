#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

using namespace tinc;

TEST(PythonClient, ParameterSpaces) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"param_space"};
  auto ps_dim = ps.newDimension("ps_dim");
  ps_dim->getParameter<al::Parameter>().min(1.3);
  ps_dim->getParameter<al::Parameter>().max(2.53);
  ps_dim->getParameter<al::Parameter>().set(1.99);
  auto ps_dim_reply = ps.newDimension("ps_dim_reply");
  ps_dim_reply->getParameter<al::Parameter>().min(2.3);
  ps_dim_reply->getParameter<al::Parameter>().max(3.53);
  ps_dim_reply->getParameter<al::Parameter>().set(2.88);
  tserver << ps;

  std::string pythonCode = R"(
#tclient.debug = True
tclient.request_parameter_spaces()
time.sleep(0.5)
test_output = [parameter_space_to_dict(ps) for ps in
tclient.parameter_spaces]

tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  tserver.stop();

  auto output = ptest.readResults();

  EXPECT_EQ(output.size(), 1);

  auto out_ps = output[0];

  EXPECT_EQ(out_ps["id"], "param_space");
  auto params = out_ps["_parameters"];

  EXPECT_EQ(params.size(), 2);

  EXPECT_EQ(params[0]["id"], "ps_dim");
  EXPECT_FLOAT_EQ(params[0]["_minimum"], 1.3f);
  EXPECT_FLOAT_EQ(params[0]["_maximum"], 2.53f);
  EXPECT_FLOAT_EQ(params[0]["_value"], 1.99f);

  EXPECT_EQ(params[1]["id"], "ps_dim_reply");
  EXPECT_FLOAT_EQ(params[1]["_minimum"], 2.3f);
  EXPECT_FLOAT_EQ(params[1]["_maximum"], 3.53f);
  EXPECT_FLOAT_EQ(params[1]["_value"], 2.88f);
}

TEST(PythonClient, ParameterSpacesRTCallback) {
  TincServer tserver;
  // tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"param_space"};
  auto dim = ps.newDimension("dim");
  auto dim2 = ps.newDimension("dim2");
  tserver << ps;

  std::string pythonCode = R"(

while len(tclient.parameter_spaces) == 0 :
    time.sleep(0.1)

ps = tclient.get_parameter_space("param_space")

while not tclient.parameter_spaces[0].get_parameter('dim'):
    time.sleep(0.1)

def cb(value):
    ps.get_parameter('dim2').set_value(value * 2)

# Connect ps_dim to ps_dim_reply
ps.get_parameter("dim").register_callback(cb)

#tclient.debug = True
#tclient.print()

while ps.get_parameter("dim2").value != 4:
    time.sleep(0.2)
test_output = [parameter_to_dict(p) for p in
tclient.parameters]

tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  std::thread th([&]() { ptest.runPython(pythonCode); });

  int counter = 0;

  while (tserver.connectionCount() == 0) {
    al::al_sleep(0.05);
    if (counter++ > 2000) {
      std::cerr << "Connection Timeout" << std::endl;
      break;
    }
  }

  std::cout << "Connected" << std::endl;
  al::al_sleep(1);

  dim->setCurrentValue(2.0);

  while (dim2->getCurrentValue() != 4.0) {
    al::al_sleep(0.05);
  }

  if (th.joinable())
    th.join();

  tserver.stop();

  auto output = ptest.readResults();
  EXPECT_EQ(output.size(), 2);

  EXPECT_EQ(output[0]["id"], "dim");
  EXPECT_FLOAT_EQ(output[0]["_value"], 2.0f);
  EXPECT_EQ(output[1]["id"], "dim2");
  EXPECT_FLOAT_EQ(output[1]["_value"], 4.0f);
}

TEST(PythonClient, ParameterSpacesRT) {
  TincServer tserver;
  // tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"param_space"};
  auto ps_dim = ps.newDimension("ps_dim");
  auto ps_dim_reply =
      ps.newDimension("ps_dim_reply"); // Python will return values here.
  tserver << ps;

  std::string pythonCode = R"(
#tclient.debug = True
time.sleep(0.3)

while len(tclient.parameter_spaces) == 0 :
    time.sleep(0.1)

ps = tclient.get_parameter_space("param_space")

while not tclient.parameter_spaces[0].get_parameter('ps_dim_reply'):
    time.sleep(0.1)

def cb(value):
    print(f"got value {value}")
    ps.get_parameter('ps_dim_reply').set_value(value * 2)

# Connect ps_dim to ps_dim_reply
ps.get_parameter("ps_dim").register_callback(cb)

tclient.debug = True

# Notify I'm ready:
time.sleep(0.1)
print("python: reply 10.0")
ps.get_parameter('ps_dim_reply').value = 10.0

print(ps.get_parameter("ps_dim").__dict__)
print("python: waiting for ps_dim == 1.0")
while abs(tclient.get_parameter("ps_dim").value - 1.0) > 0.00001:
    time.sleep(0.05)


time.sleep(0.05)
print("python: waiting for ps_dim == 100")
while tclient.get_parameter("ps_dim").value != 100:
    print(tclient.get_parameter("ps_dim").value)
    time.sleep(0.05)

print("python: done waiting for ps_dim == 100")
tclient.stop()
)";
  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  std::thread th([&]() { ptest.runPython(pythonCode); });

  int counter = 0;

  al::al_sleep(2.0);
  while (tserver.connectionCount() == 0) {
    al::al_sleep(0.05);
    if (counter++ > 2000) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  // Python will send 10.0 on ps_dim_reply when ready
  counter = 0;
  while (fabs(ps_dim_reply->getCurrentValue() - 10.0) > 0.0001) {
    al::al_sleep(0.05);
    if (counter++ > 50) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }
  EXPECT_FLOAT_EQ(ps_dim_reply->getCurrentValue(), 10.0);

  std::cout << "C++ Send ps_dim 1.0" << std::endl;
  ps_dim->setCurrentValue(1.0);

  counter = 0;
  while (fabs(ps_dim_reply->getCurrentValue() - 2.0) > 0.00001) {
    al::al_sleep(0.05);
    if (counter++ > 50) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }
  EXPECT_FLOAT_EQ(ps_dim_reply->getCurrentValue(), 2.0);

  std::cout << "C++ Send ps_dim 100" << std::endl;
  ps_dim->setCurrentValue(100);

  counter = 0;
  while (tserver.connectionCount() > 0) {
    al::al_sleep(0.05);
    if (counter++ > 50) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  if (th.joinable())
    th.join();

  tserver.stop();
}

TEST(PythonClient, ParameterSpace_Sweep) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"param_space"};

  auto dim1 = ps.newDimension("dim1");
  auto dim2 = ps.newDimension("dim2", ParameterSpaceDimension::INDEX);
  auto dim3 = ps.newDimension("dim3", ParameterSpaceDimension::ID);
  float dim1Values[5] = {0.1, 0.2, 0.3, 0.4};
  dim1->setSpaceValues(dim1Values, 4);
  dim1->conformSpace();

  float dim2Values[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
  dim2->setSpaceValues(dim2Values, 5, "xx");
  dim2->conformSpace();

  float dim3Values[6];
  std::vector<std::string> ids;
  for (int i = 0; i < 6; i++) {
    dim3Values[i] = i * 0.01;
    ids.push_back("id" + std::to_string(i));
  }
  dim3->setSpaceValues(dim3Values, 6);
  dim3->setSpaceIds(ids);
  dim3->conformSpace();

  ps.setCurrentPathTemplate("file_%%dim1%%_%%dim2%%");
  tserver << ps;

  std::string pythonCode = R"(
#tclient.debug = True
import time
time.sleep(0.1)
tclient.request_parameter_spaces()
while len(tclient.parameter_spaces) == 0:
    time.sleep(0.1)

time.sleep(0.1)

ps = tclient.get_parameter_space("param_space")

def proc(dim1, dim2, dim3):
    print(f"sweep {dim1} {dim2} {dim3}")
    return dim1*dim2*dim3

ps.enable_cache("python_cache_test")
ps.sweep(proc)

test_output = [parameter_space_to_dict(ps) for ps in
tclient.parameter_spaces]

tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  tserver.stop();

  EXPECT_TRUE(al::File::isDirectory("python_cache_test"));
  auto dirEntries = al::itemListInDir("python_cache_test");

  EXPECT_EQ(dirEntries.count(), 4 * 5 * 6);

  auto output = ptest.readResults();
}
