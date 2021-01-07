#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(DataPool, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.5); // Give time to connect

  tclient.requestDataPools();

  al::al_sleep(0.5); // Give time to connect

  // TODO check that the parameter space details are correct

  tclient.stop();
  tserver.stop();
}
