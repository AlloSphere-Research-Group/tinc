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

// TEST(TincProtocol, ParameterBool) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   al::ParameterBool p{"param", "group", true};
//   tserver << p;

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   // if p was registered after client does handshake, this isn't needed
//   tclient.requestParameters();
//   al::al_sleep(0.5); // wait for parameters to get sent

//   auto *param = tclient.getParameter("param");
//   EXPECT_NE(param, nullptr);

//   auto *paramBool = static_cast<al::ParameterBool *>(param);
//   EXPECT_EQ(paramBool->get(), true);

//   // change value on the serverside
//   p.set(false);
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(paramBool->get(), false);

//   // change value on the clientside
//   paramBool->set(true);
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(p.get(), true);

//   tclient.stop();
//   tserver.stop();
// }

// TEST(TincProtocol, ParameterString) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   al::ParameterString p{"param", "group", "default"};
//   tserver << p;

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   // if p was registered after client does handshake, this isn't needed
//   tclient.requestParameters();
//   al::al_sleep(0.5); // wait for parameters to get sent

//   auto *param = tclient.getParameter("param");
//   EXPECT_NE(param, nullptr);

//   auto *paramString = static_cast<al::ParameterString *>(param);
//   EXPECT_EQ(paramString->get(), "default");

//   // change value on the serverside
//   p.set("value");
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(paramString->get(), "value");

//   // change value on the clientside
//   paramString->set("newValue");
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(p.get(), "newValue");

//   tclient.stop();
//   tserver.stop();
// }

// TEST(TincProtocol, ParameterInt) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   al::ParameterInt p{"param", "group", 3, -10, 11};
//   tserver << p;

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   // if p was registered after client does handshake, this isn't needed
//   tclient.requestParameters();
//   al::al_sleep(0.5); // wait for parameters to get sent

//   auto *param = tclient.getParameter("param");
//   EXPECT_NE(param, nullptr);

//   auto *paramInt = static_cast<al::ParameterInt *>(param);
//   EXPECT_EQ(paramInt->min(), -10);
//   EXPECT_EQ(paramInt->max(), 11);
//   EXPECT_EQ(paramInt->get(), 3);

//   // change value on the serverside
//   p.set(4);
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(paramInt->get(), 4);

//   // change value on the clientside
//   paramInt->set(5);
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(p.get(), 5);

//   tclient.stop();
//   tserver.stop();
// }

TEST(TincProtocol, ParameterVec3) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterVec3 p{"param", "group", al::Vec3f(1, 2, 3)};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  // if p was registered after client does handshake, this isn't needed
  tclient.requestParameters();
  al::al_sleep(0.5); // wait for parameters to get sent

  auto *param = tclient.getParameter("param");
  EXPECT_NE(param, nullptr);

  auto *paramVec3 = static_cast<al::ParameterVec3 *>(param);
  EXPECT_EQ(paramVec3->get(), al::Vec3f(1, 2, 3));

  // // change value on the serverside
  p.set(al::Vec3f(4, 5, 6));
  al::al_sleep(0.5); // wait for new value

  EXPECT_EQ(paramVec3->get(), al::Vec3f(4, 5, 6));

  // change value on the clientside
  paramVec3->set(al::Vec3f(7, 8, 9));
  al::al_sleep(0.5); // wait for new value

  EXPECT_EQ(p.get(), al::Vec3f(7, 8, 9));

  tclient.stop();
  tserver.stop();
}

// TEST(TincProtocol, ParameterVec4) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterColor) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterPose) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterMenu) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterChoice) { EXPECT_TRUE(false); }

// TEST(TincProtocol, ParameterTrigger) { EXPECT_TRUE(false); }