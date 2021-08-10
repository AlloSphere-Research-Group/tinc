#include "gtest/gtest.h"

#include "tinc/DistributedPath.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/math/al_Random.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

using namespace tinc;

TEST(PythonClient, CacheDirectory) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps("ps");
  tserver << ps;
  ps.setRootPath("cache_root");
  ps.enableCache("cache_test");

  std::string pythonCode = R"(
import time

#tclient.debug = True
tclient.request_parameter_spaces()

while tclient.get_parameter_space("ps") == None:
    time.sleep(0.1)

# Wait for things to settle
ps = tclient.get_parameter_space("ps")
time.sleep(0.5)
test_output = [ps._cache_manager._cache_root, ps._cache_manager._cache_dir,  ps._cache_manager._metadata_file ]

#time.sleep(0.1)
tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  auto output = ptest.readResults();

  EXPECT_EQ(output.size(), 3);

  EXPECT_EQ(output[0], ps.getRootPath());
  EXPECT_EQ(output[1], ps.getCacheManager()->getDistributedPath().relativePath);
  EXPECT_EQ(output[2], ps.getCacheManager()->getDistributedPath().filename);
  tserver.stop();
}
