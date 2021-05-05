#include "gtest/gtest.h"

#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDFData.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/math/al_Random.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

#include "python_common.hpp"

using namespace tinc;

TEST(PythonClient, DiskbufferNetcdf) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  DiskBufferNetCDFData ncBuffer{"nc", "test.nc", "python_db"};

  tserver << ncBuffer;

  NetCDFData data;
  data.setType(NetCDFTypes::FLOAT);

  auto &v = data.getVector<float>();

  int elementCount = 2048;
  for (int i = 0; i < elementCount; i++) {
    v.push_back(al::rnd::uniform());
  }

  ncBuffer.setData(data);

  std::string pythonCode = R"(
import time

tclient.request_disk_buffers()

time.sleep(0.5)

db = tclient.get_disk_buffer("nc")

#print(db.data)
test_output = [db.get_path(), db.get_base_filename(), db.get_current_filename(), db.data.tolist()]

#print(type(db.data))
#print(type(db.data[0]))
time.sleep(0.1)
tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  al::al_sleep(0.5);

  auto output = ptest.readResults();

  EXPECT_EQ(output.size(), 4);

  EXPECT_EQ(output[0], al::File::currentPath() + ncBuffer.getPath());
  EXPECT_EQ(output[1], ncBuffer.getBaseFileName());
  EXPECT_EQ(output[2], ncBuffer.getCurrentFileName());

  EXPECT_EQ(output[3].size(), elementCount);
  for (int i = 0; i < elementCount; i++) {
    EXPECT_EQ(output[3][i], v[i]);
  }
  tserver.stop();
}
