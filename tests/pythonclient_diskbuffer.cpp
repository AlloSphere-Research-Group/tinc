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

TEST(PythonClient, DiskbufferImage) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  DiskBufferImage imageBuffer{"image", "image.png", "python_db"};

  imageBuffer.setDocumentation("im buffer");
  tserver << imageBuffer;

  std::vector<unsigned char> mPixels;

  int w = 6, h = 5;
  mPixels.resize(w * h * 4);
  //  unsigned char *pixs = mPixels.data();
  for (size_t i = 0; i < mPixels.size(); i++) {
    mPixels[i] = 255 * i / (float)mPixels.size();
  }

  imageBuffer.writePixels(mPixels.data(), w, h, 4);

  std::string pythonCode = R"(
import time

#tclient.debug = True
tclient.request_disk_buffers()

while tclient.get_disk_buffer("image") == None:
    time.sleep(0.1)

db = tclient.get_disk_buffer("image")

while db.get_current_filename() == '':
    time.sleep(0.1)

initial_file = db.get_current_filename()

im = db.data
w = im.width
h = im.height
print(w,h)

match = True
for i,b in enumerate(im.tobytes()):
    if b != int(255 * (i/(w * h * 4))):
        print(f'index: {i} failed: {b} != {int(255 * (i/w * h * 4))}')
        match = False
        break

new_w = 8
new_h = 9
pixels = [[[255* j/new_w, 255* j/new_w,255*  i/new_h, 255* i/new_h] for j in range(new_w)] for i in range(new_h)]

db.write_pixels(pixels)

test_output = [db.get_relative_path(), db.get_base_filename(),initial_file, w, h, match, db.documentation]
#print(pixels)

time.sleep(0.1)
tclient.stop()
)";

  PythonTester ptest;
  ptest.pythonExecutable = PYTHON_EXECUTABLE;
  ptest.pythonModulePath = TINC_TESTS_SOURCE_DIR "/../tinc-python/tinc-python";
  ptest.runPython(pythonCode);

  auto output = ptest.readResults();

  EXPECT_EQ(output.size(), 7);

  EXPECT_EQ(output[0], imageBuffer.getRelativePath());
  EXPECT_EQ(output[1], imageBuffer.getBaseFileName());
  EXPECT_EQ(output[2], imageBuffer.getCurrentFileName());
  EXPECT_EQ(output[3], w);
  EXPECT_EQ(output[4], h);
  EXPECT_TRUE(output[5]);
  EXPECT_EQ(output[6], imageBuffer.getDocumentation());

  al::al_sleep(0.5); // wait for image from python
  auto written = imageBuffer.get();
  EXPECT_EQ(written->width(), 8);
  EXPECT_EQ(written->height(), 9);
  EXPECT_EQ(written->mArray.size(), 9 * 8 * 4);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 8; j++) {
      EXPECT_FLOAT_EQ(written->mArray[i * 8 * 4 + j * 4],
                      uint8_t(255 * j / 8.0));
      EXPECT_FLOAT_EQ(written->mArray[i * 8 * 4 + j * 4 + 1],
                      uint8_t(255 * j / 8.0));
      EXPECT_FLOAT_EQ(written->mArray[i * 8 * 4 + j * 4 + 2],
                      uint8_t(255 * i / 9.0));
      EXPECT_FLOAT_EQ(written->mArray[i * 8 * 4 + j * 4 + 3],
                      uint8_t(255 * i / 9.0));
    }
  }
  tserver.stop();
}

TEST(PythonClient, DiskbufferNetcdf) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  DiskBufferNetCDFData ncBuffer{"nc", "test.nc", "python_db"};

  ncBuffer.setDocumentation("nc buffer");
  tserver << ncBuffer;

  NetCDFData data;
  data.setType(al::VariantType::VARIANT_FLOAT);

  auto &v = data.getVector<float>();

  int elementCount = 2048;
  for (int i = 0; i < elementCount; i++) {
    v.push_back(al::rnd::uniform());
  }

  ncBuffer.setData(data);

  std::string pythonCode = R"(
import time

tclient.request_disk_buffers()

while tclient.get_disk_buffer("nc") == None:
    time.sleep(0.1)

db = tclient.get_disk_buffer("nc")

olddata = db.data.tolist()

db.data = [0,1,2,3,4]
db.data = [5,6,7,8]
db.data = [0,1,2,3,4,5]
db.data = [5,6,7,8, 9]
#print(db.data)
test_output = [db.get_relative_path(), db.get_base_filename(), db.get_current_filename(), olddata, db.documentation]

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

  EXPECT_EQ(output.size(), 5);

  EXPECT_EQ(output[0], ncBuffer.getRelativePath());
  EXPECT_EQ(output[1], ncBuffer.getBaseFileName());
  EXPECT_EQ(output[2], ncBuffer.getCurrentFileName());

  EXPECT_EQ(output[3].size(), elementCount);
  for (int i = 0; i < elementCount; i++) {
    EXPECT_EQ(output[3][i], v[i]);
  }
  EXPECT_EQ(output[4], ncBuffer.getDocumentation());

  auto &newData = ncBuffer.get()->getVector<float>();
  EXPECT_EQ(newData, std::vector<float>({5, 6, 7, 8, 9}));
  tserver.stop();
}
