#include "gtest/gtest.h"

#include "tinc/ProcessorCpp.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(Status, WaitForServer) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int sharedValue = 0;
  ProcessorCpp proc{"proc"};
  ParameterSpaceDimension dim{"dim"};

  proc.processingFunction = [&]() { return true; };

  al::al_sleep(0.1);

  tserver << proc << dim;
  EXPECT_TRUE(tclient.waitForServer());

  EXPECT_EQ(tclient.serverStatus(), TincProtocol::Status::STATUS_AVAILABLE);

  tserver.markBusy();
  al::al_sleep(0.1);
  EXPECT_EQ(tclient.serverStatus(), TincProtocol::Status::STATUS_BUSY);

  tserver.markAvailable();
  al::al_sleep(0.1);
  EXPECT_EQ(tclient.serverStatus(), TincProtocol::Status::STATUS_AVAILABLE);

  tclient.stop();
  tserver.stop();
}
