#include "gtest/gtest.h"

#include "tinc/TincServer.hpp"
#include "tinc/TincClient.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(TincProtocol, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(1.0); // Give time to connect
  EXPECT_EQ(tserver.connectionCount(), 1);
  EXPECT_TRUE(tclient.isConnected());

  tserver.stop();
  tclient.stop();
}

TEST(TincProtocol, MultiConnection) {
  TincServer tserver;
  tserver.verbose(true);
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.3); // This should be removed
  TincClient tclient2;
  EXPECT_TRUE(tclient2.start());

  al::al_sleep(0.3); // This should be removed
  TincClient tclient3;
  EXPECT_TRUE(tclient3.start());

  al::al_sleep(0.3); // This should be removed
  TincClient tclient4;
  EXPECT_TRUE(tclient4.start());

  al::al_sleep(1.0); // Give time to connect
  EXPECT_EQ(tserver.connectionCount(), 4);
  EXPECT_TRUE(tclient.isConnected());
  EXPECT_TRUE(tclient2.isConnected());
  EXPECT_TRUE(tclient3.isConnected());
  EXPECT_TRUE(tclient4.isConnected());

  tserver.stop();
  tclient.stop();
  tclient2.stop();
  tclient3.stop();
  tclient4.stop();
}
