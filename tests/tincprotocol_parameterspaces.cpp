#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(ProtocolParameterSpace, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"param_space"};
  auto ps_dim = ps.newDimension("ps_dim");
  tserver << ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  tclient.requestParameterSpaces();

  bool timeout = false;
  int counter = 0;

  while (tclient.parameterSpaces().size() != 1 && !timeout) {
    al::al_sleep(0.001); // Give time to connect
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      timeout = true;
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  // TODO complete checks

  tclient.stop();
  tserver.stop();
}
