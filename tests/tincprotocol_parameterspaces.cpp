#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"

#include "al/ui/al_Parameter.hpp"

using namespace tinc;

TEST(ProtocolParameterSpace, Connection) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"paramspace"};
  auto ps_dim = ps.newDimension("psdim");
  EXPECT_NE(ps_dim, nullptr);
  ps.setDocumentation("param space");

  tserver << ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  auto client_ps = tclient.getParameterSpace("paramspace");

  EXPECT_NE(client_ps, nullptr);

  EXPECT_EQ(client_ps->getDocumentation(), "param space");

  // auto client_ps_dim = client_ps->getDimension("psdim");
  // auto client_dim = tclient.getParameter("psdim");

  // EXPECT_NE(client_ps_dim, nullptr);
  // EXPECT_NE(client_dim, nullptr);

  // al::al_sleep(0.2);
  // ps.removeDimension("psdim");
  // al::al_sleep(0.2);

  // counter = 0;
  // while (tclient.dimensions().size() != 0) {
  //   al::al_sleep(0.05);
  //   if (counter++ > TINC_TESTS_TIMEOUT_MS) {
  //     std::cerr << "Timeout 2" << std::endl;
  //     break;
  //   }
  // }

  // auto client_dim2 = tclient.getParameter("psdim");
  // al::al_sleep(0.2);

  // EXPECT_EQ(client_dim2, nullptr);

  tclient.stop();
  tserver.stop();
}

TEST(ProtocolParameterSpace, RootPath) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"paramspace"};
  ps.setRootPath("rootpath");

  tserver << ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  auto client_ps = tclient.getParameterSpace("paramspace");

  EXPECT_EQ(ps.getRootPath(), client_ps->getRootPath());
  // FIXME isSamePath broken for this case...
  EXPECT_TRUE(al::File::isSamePath(ps.getRootPath(), client_ps->getRootPath()));

  tclient.stop();
  tserver.stop();
}

TEST(ProtocolParameterSpace, PathTemplate) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  ParameterSpace ps{"paramspace"};
  ps.setCurrentPathTemplate("greattemplate");

  tserver << ps;

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  auto client_ps = tclient.getParameterSpace("paramspace");

  EXPECT_EQ(ps.getCurrentPathTemplate(), client_ps->getCurrentPathTemplate());
  // FIXME isSamePath broken for this case...
  EXPECT_TRUE(al::File::isSamePath(ps.getCurrentPathTemplate(),
                                   client_ps->getCurrentPathTemplate()));

  tclient.stop();
  tserver.stop();
}
