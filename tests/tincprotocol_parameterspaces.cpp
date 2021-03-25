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
  EXPECT_NE(ps_dim, nullptr);

  tserver << ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  tclient.requestParameterSpaces();

  al::al_sleep(0.2); // Give time to connect
  auto client_ps = tclient.getParameterSpace("param_space");

  int counter = 0;
  while (client_ps == nullptr) {
    al::al_sleep(0.05); // Give time to connect
    client_ps = tclient.getParameterSpace("param_space");
    if (counter++ > TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  auto client_ps_dim = client_ps->getDimension("ps_dim");
  auto client_dim = tclient.getParameter("ps_dim");

  EXPECT_NE(client_ps_dim, nullptr);
  EXPECT_NE(client_dim, nullptr);

  ps.removeDimension("ps_dim");
  al::al_sleep(0.2);

  counter = 0;
  while (tclient.dimensions().size() != 0) {
    al::al_sleep(0.05);
    if (counter++ > TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout 2" << std::endl;
      break;
    }
  }

  auto client_dim2 = tclient.getParameter("ps_dim");
  al::al_sleep(0.2);

  EXPECT_EQ(client_dim2, nullptr);

  tclient.stop();
  tserver.stop();
}