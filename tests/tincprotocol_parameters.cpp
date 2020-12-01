#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(TincProtocol, ParameterFloat) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::Parameter p{"param", "group", 0.2, -10, 9.9};
  tserver << p;
  p.set(0.5);

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramFloat = static_cast<al::Parameter *>(param);
  EXPECT_FLOAT_EQ(paramFloat->min(), -10);
  EXPECT_FLOAT_EQ(paramFloat->max(), 9.9);
  EXPECT_FLOAT_EQ(paramFloat->get(), 0.5);

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterBool) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterBool p{"param", "group", true};
  tserver << p;

  p.set(1.0);
  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramBool = static_cast<al::ParameterBool *>(param);
  EXPECT_EQ(paramBool->getDefault(), 0.0);
  EXPECT_EQ(paramBool->get(), 1.0);

  // change value on the serverside
  p.set(false);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramBool->get(), false);

  // change value on the clientside
  paramBool->set(true);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), true);

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterString) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterString p{"param", "group", "default"};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramString = static_cast<al::ParameterString *>(param);
  EXPECT_EQ(paramString->getDefault(), "default");
  EXPECT_EQ(paramString->get(), "default");

  // change value on the serverside
  p.set("value");
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramString->get(), "value");

  // change value on the clientside
  paramString->set("newValue");
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), "newValue");

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterInt) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterInt p{"param", "group", 3, -10, 11};
  p.setNoCalls(-3);
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramInt = static_cast<al::ParameterInt *>(param);
  EXPECT_EQ(paramInt->min(), -10);
  EXPECT_EQ(paramInt->max(), 11);
  EXPECT_EQ(paramInt->getDefault(), 3);
  EXPECT_EQ(paramInt->get(), -3);

  // change value on the serverside
  p.set(4);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramInt->get(), 4);

  // change value on the clientside
  paramInt->set(5);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), 5);

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterVec3) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterVec3 p{"param", "group", al::Vec3f(1, 2, 3)};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramVec3 = static_cast<al::ParameterVec3 *>(param);
  EXPECT_EQ(paramVec3->getDefault(), al::Vec3f(1, 2, 3));
  EXPECT_EQ(paramVec3->get(), al::Vec3f(1, 2, 3));

  // change value on the serverside
  p.set(al::Vec3f(4, 5, 6));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramVec3->get(), al::Vec3f(4, 5, 6));

  // change value on the clientside
  paramVec3->set(al::Vec3f(7, 8, 9));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), al::Vec3f(7, 8, 9));

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterVec4) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterVec4 p{"param", "group", al::Vec4f(1, 2, 3, 4)};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramVec4 = static_cast<al::ParameterVec4 *>(param);
  EXPECT_EQ(paramVec4->getDefault(), al::Vec4f(1, 2, 3, 4));
  EXPECT_EQ(paramVec4->get(), al::Vec4f(1, 2, 3, 4));

  // change value on the serverside
  p.set(al::Vec4f(4, 5, 6, 7));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramVec4->get(), al::Vec4f(4, 5, 6, 7));

  // change value on the clientside
  paramVec4->set(al::Vec4f(7, 8, 9, 10));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), al::Vec4f(7, 8, 9, 10));

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterColor) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterColor p{"param", "group", al::Color(0.1f, 0.2f, 0.3f, 0.4f)};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramColor = static_cast<al::ParameterColor *>(param);
  EXPECT_EQ(paramColor->getDefault(), al::Color(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_EQ(paramColor->get(), al::Color(0.1f, 0.2f, 0.3f, 0.4f));

  // change value on the serverside
  p.set(al::Color(0.4f, 0.5f, 0.6f, 0.7f));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramColor->get(), al::Color(0.4f, 0.5f, 0.6f, 0.7f));

  // change value on the clientside
  paramColor->set(al::Color(0.7f, 0.8f, 0.9f, 1.f));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), al::Color(0.7f, 0.8f, 0.9f, 1.f));

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterPose) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  // FIXME why are ftns like quat::getRotationTo declared static?

  // in actual use quat values should be normalized
  al::ParameterPose p{"param", "group",
                      al::Pose({0.1, 0.2, 0.3}, {0.4, 0.5, 0.6, 0.7})};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramPose = static_cast<al::ParameterPose *>(param);
  EXPECT_EQ(paramPose->getDefault(),
            al::Pose({0.1, 0.2, 0.3}, {0.4, 0.5, 0.6, 0.7}));
  EXPECT_EQ(paramPose->get(), al::Pose({0.1, 0.2, 0.3}, {0.4, 0.5, 0.6, 0.7}));

  // change value on the serverside
  p.set(al::Pose({-0.1, -0.2, -0.3}, {-0.4, -0.5, -0.6, -0.7}));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramPose->get(),
            al::Pose({-0.1, -0.2, -0.3}, {-0.4, -0.5, -0.6, -0.7}));

  // change value on the clientside
  paramPose->set(al::Pose({1.1, 1.2, 1.3}, {1.4, 1.5, 1.6, 1.7}));
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), al::Pose({1.1, 1.2, 1.3}, {1.4, 1.5, 1.6, 1.7}));

  tclient.stop();
  tserver.stop();
}

// FIXME do we share the elements too
// FIXME set min max based on element size?
TEST(TincProtocol, ParameterMenu) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::ParameterMenu p{"param", "group", 1};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramMenu = static_cast<al::ParameterMenu *>(param);
  EXPECT_EQ(paramMenu->getDefault(), 1);
  EXPECT_EQ(paramMenu->get(), 1);

  // change value on the serverside
  p.set(2);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramMenu->get(), 2);

  // change value on the clientside
  paramMenu->set(3);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), 3);

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ParameterChoice) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  // FIXME what is UINT64_C(1) ??
  // FIXME ParamterValue: why is uint32 using int32 and similar
  // FIXME ParameterChoice: why setnocalls on constructor?
  uint64_t value = 0x123456789ABC1234;
  al::ParameterChoice p{"param", "group", value};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramChoice = static_cast<al::ParameterChoice *>(param);
  EXPECT_EQ(paramChoice->getDefault(), 0x123456789ABC1234);
  EXPECT_EQ(paramChoice->get(), 0x123456789ABC1234);

  // change value on the serverside
  p.set(0x23456789ABC12341);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramChoice->get(), 0x23456789ABC12341);

  // change value on the clientside
  paramChoice->set(0x3456789ABC123412);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), 0x3456789ABC123412);

  tclient.stop();
  tserver.stop();
}

// FIXME why is ParamterBool float but Trigger bool?
TEST(TincProtocol, ParameterTrigger) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  al::Trigger p{"param", "group"};
  tserver << p;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  int counter = 0;

  al::ParameterMeta *param{nullptr};
  tclient.requestParameters();
  while (!param) {
    al::al_sleep(0.5); // Give time to connect
    param = tclient.getParameter("param", "group");
    if (counter++ == TINC_TESTS_TIMEOUT_MS) {
      std::cerr << "Timeout" << std::endl;
      break;
    }
  }

  EXPECT_NE(param, nullptr);

  auto *paramTrigger = static_cast<al::Trigger *>(param);
  EXPECT_EQ(paramTrigger->getDefault(), false);
  EXPECT_EQ(paramTrigger->get(), false);

  // change value on the serverside
  p.trigger();
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(paramTrigger->get(), true);

  // change value on the clientside
  paramTrigger->set(false);
  al::al_sleep(0.1); // wait for new value

  EXPECT_EQ(p.get(), false);

  tclient.stop();
  tserver.stop();
}

// FIXME simplify the createconfigureparameterXXXmessage
// FIXME review default
