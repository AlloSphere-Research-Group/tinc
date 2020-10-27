#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

// TEST(TincProtocol, ParameterFloat) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   al::Parameter p{"param", "group", 0.2, -10, 9.9};
//   tserver << p;
//   p.set(0.5);

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   al::al_sleep(0.5); // Give time to connect

//   tclient.requestParameters();

//   al::al_sleep(0.5); // Give time to connect

//   auto *param = tclient.getParameter("param");
//   EXPECT_NE(param, nullptr);

//   auto *paramFloat = static_cast<al::Parameter *>(param);
//   EXPECT_FLOAT_EQ(paramFloat->min(), -10);
//   EXPECT_FLOAT_EQ(paramFloat->max(), -9.9);
//   EXPECT_FLOAT_EQ(paramFloat->get(), 0.5);

//   tclient.stop();
//   tserver.stop();
// }

// TEST(TincProtocol, ParameterString) { EXPECT_TRUE(false); }

TEST(TincProtocol, ParameterInt) {
  TincServer tserver;
  tserver.setVerbose(false);
  EXPECT_TRUE(tserver.start());

  al::ParameterInt p{"param", "group", 3, -10, 11};
  tserver << p;
  // p.set(4);

  TincClient tclient;
  tclient.setVerbose(false);
  EXPECT_TRUE(tclient.start());

  tclient.requestParameters();

  al::al_sleep(0.5);

  auto *param = tclient.getParameter("param");
  EXPECT_NE(param, nullptr);

  auto *paramInt = static_cast<al::ParameterInt *>(param);
  EXPECT_EQ(paramInt->min(), -10);
  EXPECT_EQ(paramInt->max(), 11);
  // EXPECT_EQ(paramInt->get(), 4);

  // paramInt->set(5);

  // al::al_sleep(0.5);

  // EXPECT_EQ(p.get(), 5);

  // al::ParameterInt q{"param2", "group", 6, -8, 10};
  // tclient << q;
  // q.set(7);

  // tclient.sendParameters();

  // al::al_sleep(0.5);

  // auto *param2 = tserver.getParameter("param2");
  // EXPECT_NE(param2, nullptr);

  tclient.stop();
  tserver.stop();
}

// TEST(TincProtocol, ParameterVec3) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterVec4) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterColor) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterMenu) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterChoice) { EXPECT_TRUE(false); }
