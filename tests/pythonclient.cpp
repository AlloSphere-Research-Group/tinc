#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

using namespace tinc;

TEST(PythonClient, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  std::string pythonCode = R"(
time.sleep(1.0)
tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  std::thread th([&]() { ptest.runPython(pythonCode); });
  int counter = 0;
  while (tserver.connectionCount() == 0) {
    al::al_sleep(0.05);
    if ((counter * 50) >= TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
    counter++;
  }
  EXPECT_EQ(tserver.connectionCount(), 1);
  th.join();

  EXPECT_EQ(tserver.connectionCount(), 0);

  tserver.stop();
}

// FIXME simplify the createconfigureparameterXXXmessage
// FIXME review default
