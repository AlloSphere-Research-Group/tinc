#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(TincProtocol, ParameterSpaces) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::Parameter p{"param", "group", 0.2, -10, 9.9};
  tserver << p;
  p.set(0.5);

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.5); // Give time to connect

  tclient.requestParameterSpaces();

  al::al_sleep(0.5); // Give time to connect

  // TODO check that the parameter space details are correct

  tclient.stop();
  tserver.stop();
}
