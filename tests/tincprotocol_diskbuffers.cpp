#include "gtest/gtest.h"

#include "tinc/DiskBuffer.hpp"
#include "tinc/DiskBufferImage.hpp"
#include "tinc/DiskBufferJson.hpp"
#include "tinc/DiskBufferNetCDFData.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/math/al_Random.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(DiskBuffer, Connection) {
  TincServer tserver;
  // tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  // TODO create disk buffers of different types
  DiskBufferImage imageBuffer{"image", "test.png"};
  // DiskBufferJson jsonBuffer{"json", "test.json"};
  // DiskBufferNetCDFDouble netcdfBuffer{"nc", "test.nc"};

  auto imageName = imageBuffer.getCurrentFileName();

  // generating example image
  std::vector<unsigned char> pix;
  for (size_t i = 0; i < 9; i++) {
    al::Colori c = al::HSV(al::rnd::uniform(), 1.0, 1.0);
    pix.push_back(c.r);
    pix.push_back(c.g);
    pix.push_back(c.b);
  }

  al::Image::saveImage(imageName, pix.data(), 3, 3);

  // update the buffer with the new data
  imageBuffer.loadData(imageName);

  tserver << imageBuffer;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());
  // tclient.setVerbose(true);

  bool timeout = false;
  int counter = 0;

  al::ParameterMeta *param{nullptr};
  while (!tclient.isConnected() && !timeout) {
    al::al_sleep(0.001); // Give time to connect
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_TRUE(tclient.isConnected());

  tclient.requestDiskBuffers();

  al::al_sleep(0.5); // Give time to connect

  // TODO check that the disk buffer details are correct

  tclient.stop();
  tserver.stop();
}
