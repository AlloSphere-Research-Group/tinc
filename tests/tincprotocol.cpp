#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(TincProtocol, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

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

  EXPECT_EQ(tserver.connectionCount(), 4);
  EXPECT_TRUE(tclient.isConnected());
  EXPECT_TRUE(tclient2.isConnected());
  EXPECT_TRUE(tclient3.isConnected());
  EXPECT_TRUE(tclient4.isConnected());

  tclient.stop();
  tclient2.stop();
  tclient3.stop();
  tclient4.stop();
  tserver.stop();
}
