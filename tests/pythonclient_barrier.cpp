#include "gtest/gtest.h"

#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

#include <thread>

using namespace tinc;

TEST(PythonClient, Barrier) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  std::string pythonCode = R"(
import time

tclient.barrier()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  std::thread th([&]() { ptest.runPython(pythonCode); });

  while (tserver.connectionCount() == 0) {
    al::al_sleep(0.1);
  }

  tserver.barrier();

  th.join();
  tserver.stop();
}
