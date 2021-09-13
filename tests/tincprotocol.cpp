#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(TincProtocol, Ping) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  //  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());
  uint64_t pingCode;
  for (int i = 0; i < 100; i++) {
    pingCode = tclient.pingServer();
  }
  EXPECT_TRUE(tclient.waitForPing(pingCode, 5));

  for (int i = 0; i < 100; i++) {
    pingCode = tclient.pingServer();
    EXPECT_TRUE(tclient.waitForPing(pingCode, 5));
  }

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, Connection) {
  TincServer tserver;
  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.1);

  EXPECT_EQ(tserver.connectionCount(), 1);
  EXPECT_TRUE(tclient.isConnected());

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, MultiConnection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  TincClient tclient2;
  EXPECT_TRUE(tclient2.start());

  TincClient tclient3;
  EXPECT_TRUE(tclient3.start());

  TincClient tclient4;
  EXPECT_TRUE(tclient4.start());

  al::al_sleep(1.0);

  EXPECT_EQ(tserver.connectionCount(), 4);
  EXPECT_TRUE(tclient.isConnected());
  EXPECT_TRUE(tclient2.isConnected());
  EXPECT_TRUE(tclient3.isConnected());
  EXPECT_TRUE(tclient4.isConnected());

  tclient.stop();
  al::al_sleep(0.3);

  EXPECT_EQ(tserver.connectionCount(), 3);
  EXPECT_TRUE(!tclient.isConnected());
  EXPECT_TRUE(tclient2.isConnected());
  EXPECT_TRUE(tclient3.isConnected());
  EXPECT_TRUE(tclient4.isConnected());

  tclient2.stop();
  al::al_sleep(0.3);

  EXPECT_EQ(tserver.connectionCount(), 2);
  EXPECT_TRUE(!tclient.isConnected());
  EXPECT_TRUE(!tclient2.isConnected());
  EXPECT_TRUE(tclient3.isConnected());
  EXPECT_TRUE(tclient4.isConnected());

  tclient3.stop();
  al::al_sleep(0.3);

  EXPECT_EQ(tserver.connectionCount(), 1);
  EXPECT_TRUE(!tclient.isConnected());
  EXPECT_TRUE(!tclient2.isConnected());
  EXPECT_TRUE(!tclient3.isConnected());
  EXPECT_TRUE(tclient4.isConnected());

  tclient4.stop();
  al::al_sleep(0.5);
  EXPECT_EQ(tserver.connectionCount(), 0);
  EXPECT_TRUE(!tclient.isConnected());
  EXPECT_TRUE(!tclient2.isConnected());
  EXPECT_TRUE(!tclient3.isConnected());
  EXPECT_TRUE(!tclient4.isConnected());
  tserver.stop();
}
