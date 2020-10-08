#include "gtest/gtest.h"

#include "tinc/TincServer.hpp"
#include "tinc/TincClient.hpp"

#include "al/system/al_Time.hpp"

using namespace tinc;

TEST(TincProtocol, RemoteParameter) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  tclient.start();
  al::al_sleep(1.0); // Give time to connect
  tclient.isConnected();

  // TODO complete.

  tclient.stop();
  tserver.stop();
}
