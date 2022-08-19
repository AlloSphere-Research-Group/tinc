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

  ProcessorCpp proc{"proc"};
  ParameterSpaceDimension dim{"dim"};

  proc.processingFunction = [&]() { return true; };

  tserver << proc << dim;
  EXPECT_TRUE(tclient.start());

  tserver.waitForConnections(1);

  tclient.waitForPong(tclient.pingServer());

  EXPECT_EQ(tclient.serverStatus(), TincProtocol::Status::STATUS_AVAILABLE);

  tserver.markBusy();

  al::al_sleep(0.2);
  EXPECT_EQ(tclient.serverStatus(), TincProtocol::Status::STATUS_BUSY);

  tserver.markAvailable();
  al::al_sleep(0.2);
  EXPECT_EQ(tclient.serverStatus(), TincProtocol::Status::STATUS_AVAILABLE);

  tclient.stop();
  tserver.stop();
}
