#include "gtest/gtest.h"

#include "tinc/DiskBuffer.hpp"
#include "tinc/ImageDiskBuffer.hpp"
#include "tinc/JsonDiskBuffer.hpp"
#include "tinc/NetCDFDiskBuffer.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/math/al_Random.hpp"
#include "al/system/al_Time.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(TincProtocol, DiskBuffers) {
  TincServer tserver;
  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  // TODO create disk buffers of different types
  ImageDiskBuffer imageBuffer{"image", "test.png"};
  // JsonDiskBuffer jsonBuffer{"json", "test.json"};
  // NetCDFDiskBufferDouble netcdfBuffer{"nc", "test.nc"};

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
  imageBuffer.updateData(imageName);

  tserver << imageBuffer;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());
  tclient.setVerbose(true);

  al::al_sleep(0.5); // Give time to connect
  EXPECT_TRUE(tclient.isConnected());

  tclient.requestDiskBuffers();

  al::al_sleep(0.5); // Give time to connect

  // TODO check that the disk buffer details are correct

  tclient.stop();
  tserver.stop();
}
