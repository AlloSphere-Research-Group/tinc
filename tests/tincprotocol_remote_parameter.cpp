#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

// TEST(TincProtocol, RemoteParameterFloat) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   TincClient tclient;
//   tclient.start();
//   al::al_sleep(1.0); // Give time to connect
//   tclient.isConnected();

//   // TODO complete.

//   tclient.stop();
//   tserver.stop();
// }

// TEST(TincProtocol, RemoteParameterBool) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   al::ParameterBool p{"param", "group", true};

//   // register automatically gets propagated to server
//   tclient << p;
//   al::al_sleep(0.5); // wait for parameters to get sent

//   auto *param = tserver.getParameter("param");
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

// TEST(TincProtocol, RemoteParameterString) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   al::ParameterString p{"param", "group", "default"};

//   // register automatically gets propagated to server
//   tclient << p;
//   al::al_sleep(0.5); // wait for parameters to get sent

//   auto *param = tserver.getParameter("param");
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

// TEST(TincProtocol, RemoteParameterInt) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   al::ParameterInt p{"param", "group", 3, -10, 11};

//   // register automatically gets propagated to server
//   tclient << p;
//   al::al_sleep(0.5); // wait for parameter to get sent

//   auto *param = tserver.getParameter("param");
//   EXPECT_NE(param, nullptr);

//   auto *paramInt = static_cast<al::ParameterInt *>(param);
//   EXPECT_EQ(paramInt->min(), -10);
//   EXPECT_EQ(paramInt->max(), 11);
//   EXPECT_EQ(paramInt->get(), 3);

//   // change value on the clientside
//   p.set(4);
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(paramInt->get(), 4);

//   // change value on the serverside
//   paramInt->set(5);
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(p.get(), 5);

//   tclient.stop();
//   tserver.stop();
// }

// TEST(TincProtocol, RemoteParameterVec3) {
//   TincServer tserver;
//   EXPECT_TRUE(tserver.start());

//   TincClient tclient;
//   EXPECT_TRUE(tclient.start());

//   al::ParameterVec3 p{"param", "group", al::Vec3f(1, 2, 3)};

//   // register automatically gets propagated to server
//   tclient << p;
//   al::al_sleep(0.5); // wait for parameters to get sent

//   auto *param = tserver.getParameter("param");
//   EXPECT_NE(param, nullptr);

//   auto *paramVec3 = static_cast<al::ParameterVec3 *>(param);
//   EXPECT_EQ(paramVec3->get(), al::Vec3f(1, 2, 3));

//   // // change value on the clientside
//   p.set(al::Vec3f(4, 5, 6));
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(paramVec3->get(), al::Vec3f(4, 5, 6));

//   // change value on the serverside
//   paramVec3->set(al::Vec3f(7, 8, 9));
//   al::al_sleep(0.5); // wait for new value

//   EXPECT_EQ(p.get(), al::Vec3f(7, 8, 9));

//   tclient.stop();
//   tserver.stop();
// }

TEST(TincProtocol, RemoteParameterVec4) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::ParameterVec4 p{"param", "group", al::Vec4f(1, 2, 3, 4)};

  // register automatically gets propagated to server
  tclient << p;
  al::al_sleep(0.5); // wait for parameters to get sent

  auto *param = tserver.getParameter("param");
  EXPECT_NE(param, nullptr);

  auto *paramVec4 = static_cast<al::ParameterVec4 *>(param);
  EXPECT_EQ(paramVec4->get(), al::Vec4f(1, 2, 3, 4));

  // // change value on the clientside
  p.set(al::Vec4f(4, 5, 6, 7));
  al::al_sleep(0.5); // wait for new value

  EXPECT_EQ(paramVec4->get(), al::Vec4f(4, 5, 6, 7));

  // change value on the serverside
  paramVec4->set(al::Vec4f(7, 8, 9, 10));
  al::al_sleep(0.5); // wait for new value

  EXPECT_EQ(p.get(), al::Vec4f(7, 8, 9, 10));

  tclient.stop();
  tserver.stop();
}

// TEST(TincProtocol, RemoteParameterColor) { EXPECT_TRUE(false); }

// TEST(TincProtocol, RemoteParameterPose) { EXPECT_TRUE(false); }

// TEST(TincProtocol, RemoteParameterMenu) { EXPECT_TRUE(false); }

// TEST(TincProtocol, RemoteParameterChoice) { EXPECT_TRUE(false); }

// TEST(TincProtocol, RemoteParameterTrigger) { EXPECT_TRUE(false); }