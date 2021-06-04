#include "gtest/gtest.h"

#include "tinc/DataPoolJson.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/math/al_Random.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

using namespace tinc;

TEST(PythonClient, DataPoolJson) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  // Set up parameter space and data pool
  ParameterSpace ps{"ps"};

  ps.setRootPath(TINC_TESTS_SOURCE_DIR "/data");

  // This internal dimension determines the index into the elements found in
  // results.json
  auto internalDim = ps.newDimension("internal");
  internalDim->setSpaceValues(
      std::vector<float>{0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7});
  auto externalDim = ps.newDimension("external", ParameterSpaceDimension::ID);
  externalDim->setSpaceValues(std::vector<float>{10.0, 10.1, 10.2});
  externalDim->setSpaceIds({"folder1", "folder2", "folder3"});

  ps.setCurrentPathTemplate("%%external%%/");

  DataPoolJson dp{"dp", ps, "sliceCache"};
  dp.registerDataFile("results.json", "internal");

  tserver << dp;

  // Set up python client code
  std::string pythonCode = R"(

while len(tclient.datapools) == 0 :
    time.sleep(0.1)

dp = tclient.get_datapool("dp")

test_output = [datapool_to_dict(dp) for dp in tclient.datapools]

tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  auto results = ptest.readResults();

  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0]["id"], "dp");
  EXPECT_EQ(results[0]["parameter_space_id"], "ps");
  EXPECT_TRUE(al::File::isSamePath(results[0]["slice_cache_dir"], "cacheDir/"));

  tserver.stop();
}
