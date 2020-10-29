#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(TincProtocol, Processors) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  // TODO create processors of different types (perhaps in separate unit tests

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.5); // Give time to connect

  tclient.requestDiskBuffers();

  al::al_sleep(0.5); // Give time to connect

  // TODO check that the processor details are correct

  tclient.stop();
  tserver.stop();
}
