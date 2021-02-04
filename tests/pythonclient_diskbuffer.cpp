#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"

#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

using namespace tinc;

TEST(PythonClient, Diskbuffer) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ImageDiskBuffer imageBuffer{"image", "test.png"};
  DiskBufferJson jsonBuffer{"json", "test.json"};

  tserver << imageBuffer << jsonBuffer;

  std::string pythonCode = R"(
import time

tclient.request_disk_buffers()

time.sleep(0.2)
test_output = [disk_buffer_to_dict(db) for db in tclient.disk_buffers]

time.sleep(0.1)
tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  auto output = ptest.readResults();

  EXPECT_EQ(output.size(), 2);
  auto p1 = output[0];

  tserver.stop();
}
