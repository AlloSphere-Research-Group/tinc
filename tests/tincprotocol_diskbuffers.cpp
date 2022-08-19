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

TEST(DiskBufferProtocol, DiskBufferImage) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  DiskBufferImage db{"db", "test.png", "db_path", "root_path"};

  // generating example image
  std::vector<unsigned char> pix;
  int w = 3;
  int h = 3;
  for (int i = 0; i < w * h; i++) {
    al::Colori c = al::HSV(al::rnd::uniform(), 1.0, 1.0);
    pix.push_back(c.r);
    pix.push_back(c.g);
    pix.push_back(c.b);
  }

  // update the buffer with the new data
  db.writePixels(pix.data(), w, h);

  db.setDocumentation("disk buffer image");

  tserver << db;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());
  tclient.synchronize();
  // tclient.setVerbose(true);

  EXPECT_TRUE(tclient.isConnected());

  auto *dbClient = tclient.getDiskBuffer("db");
  EXPECT_NE(dbClient, nullptr);
  EXPECT_EQ(dbClient->getId(), "db");
  EXPECT_EQ(dbClient->getDocumentation(), "disk buffer image");

  EXPECT_EQ(dbClient->getBaseFileName(), "test.png");
  EXPECT_TRUE(al::File::isSamePath(dbClient->getFullPath(), db.getFullPath()));
  EXPECT_EQ(dbClient->getCurrentFileName(), "test.png");
  auto imageDbData = static_cast<DiskBufferImage *>(dbClient)->get();
  auto array = imageDbData->array();
  EXPECT_EQ(array.size(), w * h * 4);

  for (int i = 0; i < w * h; i++) {

    EXPECT_EQ(array[i * 4], pix[i * 3]);
    EXPECT_EQ(array[i * 4 + 1], pix[i * 3 + 1]);
    EXPECT_EQ(array[i * 4 + 2], pix[i * 3 + 2]);
  }

  tclient.stop();
  tserver.stop();
}

TEST(DiskBufferProtocol, DiskBufferJson) {
  TincServer tserver;
  // tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  // TODO create disk buffers of different types
  DiskBufferJson db{"db", "test.json", "db_path", "root_path"};

  nlohmann::json j;
  j = {2, 4, 6, 7};
  db.setData(j);

  db.setDocumentation("disk buffer json");
  tserver << db;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());
  tclient.synchronize();
  // tclient.setVerbose(true);

  EXPECT_TRUE(tclient.isConnected());

  auto *dbClient = tclient.getDiskBuffer("db");
  EXPECT_NE(dbClient, nullptr);

  EXPECT_EQ(dbClient->getId(), "db");
  EXPECT_EQ(dbClient->getDocumentation(), "disk buffer json");

  EXPECT_EQ(dbClient->getBaseFileName(), "test.json");
  EXPECT_TRUE(al::File::isSamePath(dbClient->getFullPath(), db.getFullPath()));
  EXPECT_EQ(dbClient->getCurrentFileName(), "test.json");
  auto jsonDbData = static_cast<DiskBufferJson *>(dbClient)->get();
  EXPECT_EQ(jsonDbData->size(), j.size());
  EXPECT_EQ(jsonDbData->at(0), j[0]);
  EXPECT_EQ(jsonDbData->at(1), j[1]);
  EXPECT_EQ(jsonDbData->at(2), j[2]);
  EXPECT_EQ(jsonDbData->at(3), j[3]);

  tclient.stop();
  tserver.stop();
}

TEST(DiskBufferProtocol, DiskBufferNetCDFData) {
  TincServer tserver;
  // tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  // TODO create disk buffers of different types
  DiskBufferNetCDFData db{"db", "test.nc", "db_path", "root_path"};

  NetCDFData data;
  data.setType(al::VariantType::VARIANT_FLOAT);
  auto &v = data.getVector<float>();
  size_t size = 256;
  for (size_t i = 0; i < size; i++) {
    v.push_back(al::rnd::uniform());
  }

  db.setData(data);
  db.setDocumentation("disk buffer netcdf");

  tserver << db;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());
  tclient.synchronize();
  // tclient.setVerbose(true);

  EXPECT_TRUE(tclient.isConnected());

  auto *dbClient = tclient.getDiskBuffer("db");
  EXPECT_NE(dbClient, nullptr);

  EXPECT_EQ(dbClient->getId(), "db");
  EXPECT_EQ(dbClient->getDocumentation(), "disk buffer netcdf");
  EXPECT_EQ(dbClient->getBaseFileName(), "test.nc");
  EXPECT_TRUE(al::File::isSamePath(dbClient->getFullPath(), db.getFullPath()));
  EXPECT_EQ(dbClient->getCurrentFileName(), "test.nc");
  auto ncDbData = static_cast<DiskBufferNetCDFData *>(dbClient)->get();
  EXPECT_EQ(ncDbData->getType(), al::VariantType::VARIANT_FLOAT);

  auto &clientVector = ncDbData->getVector<float>();
  EXPECT_EQ(v, clientVector);

  tclient.stop();
  tserver.stop();
}

TEST(DiskBufferProtocol, DiskBufferRootPathMap) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  DiskBufferImage db{"db", "test.png", "db_path", "root_path"};

  tserver.setRootMapEntry("root_path", "client_root_path");
  tserver << db;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());
  tclient.synchronize();

  EXPECT_TRUE(tclient.isConnected());
  // tclient.requestDiskBuffers();

  auto *dbClient = tclient.getDiskBuffer("db");
  EXPECT_NE(dbClient, nullptr);
  if (dbClient) {
    EXPECT_EQ(dbClient->getBaseFileName(), "test.png");
    // The client is on the same host, so no path mapping
    // should occur.
    EXPECT_TRUE(
        al::File::isSamePath(dbClient->getFullPath(), db.getFullPath()));
    EXPECT_FALSE(al::File::isSamePath(dbClient->getFullPath(),
                                      "client_root_path/db_path"));
  }

  tclient.stop();
  tserver.stop();
}
