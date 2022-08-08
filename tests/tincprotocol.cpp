#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(TincProtocol, MultiConnection) {
  // FIXME this fails on Linux when using larger numbers( e.g. 100), revealing a
  // spurious issue
  for (int i = 0; i < 20; i++) {
    //    std::cout << "Pass " << i + 1 << std::endl;
    //    TincServer tserver;
    //    EXPECT_TRUE(tserver.start());

    //    TincClient tclient;
    //    EXPECT_TRUE(tclient.start());

    //    TincClient tclient2;
    //    EXPECT_TRUE(tclient2.start());

    //    TincClient tclient3;
    //    EXPECT_TRUE(tclient3.start());

    //    TincClient tclient4;
    //    EXPECT_TRUE(tclient4.start());

    //    EXPECT_EQ(tserver.connectionCount(), 4);
    //    EXPECT_TRUE(tclient.isConnected());
    //    EXPECT_TRUE(tclient2.isConnected());
    //    EXPECT_TRUE(tclient3.isConnected());
    //    EXPECT_TRUE(tclient4.isConnected());

    //    tclient.stop();
    //    al::al_sleep(0.1);

    //    EXPECT_EQ(tserver.connectionCount(), 3);
    //    EXPECT_TRUE(!tclient.isConnected());
    //    EXPECT_TRUE(tclient2.isConnected());
    //    EXPECT_TRUE(tclient3.isConnected());
    //    EXPECT_TRUE(tclient4.isConnected());

    //    tclient2.stop();
    //    al::al_sleep(0.1);

    //    EXPECT_EQ(tserver.connectionCount(), 2);
    //    EXPECT_TRUE(!tclient.isConnected());
    //    EXPECT_TRUE(!tclient2.isConnected());
    //    EXPECT_TRUE(tclient3.isConnected());
    //    EXPECT_TRUE(tclient4.isConnected());

    //    tclient3.stop();
    //    al::al_sleep(0.1);

    //    EXPECT_EQ(tserver.connectionCount(), 1);
    //    EXPECT_TRUE(!tclient.isConnected());
    //    EXPECT_TRUE(!tclient2.isConnected());
    //    EXPECT_TRUE(!tclient3.isConnected());
    //    EXPECT_TRUE(tclient4.isConnected());

    //    tclient4.stop();
    //    al::al_sleep(0.1);

    //    EXPECT_EQ(tserver.connectionCount(), 0);
    //    EXPECT_TRUE(!tclient.isConnected());
    //    EXPECT_TRUE(!tclient2.isConnected());
    //    EXPECT_TRUE(!tclient3.isConnected());
    //    EXPECT_TRUE(!tclient4.isConnected());
    //    tserver.stop();
  }
}

TEST(TincProtocol, Synchronization) {
  for (int i = 0; i < 100; i++) {
    std::cout << "pass " << i << std::endl;
    TincServer tserver;
    tserver.setVerbose(true);
    EXPECT_TRUE(tserver.start());

    TincClient tclient;
    tclient.setVerbose(true);
    EXPECT_TRUE(tclient.start());
    tclient.synchronize();
    tserver.waitForConnections(1);
    ParameterSpace ps{"ps"};
    tserver << ps;

    ps.setDocumentation("hello");

    std::thread th([&]() { tclient.barrier(); });
    tserver.barrier();
    th.join();
    tclient.waitForPong(tclient.pingServer());
    auto *ps_client = tclient.getParameterSpace("ps");
    EXPECT_EQ(ps_client->getDocumentation(), ps.getDocumentation());
    tclient.stop();
    tserver.stop();
  }
}

TEST(TincProtocol, Basic) {
  TincServer tserver;
  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());
  tclient.waitForPong(tclient.pingServer());

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, Ping) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  //  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());
  uint64_t pingCode;
  for (int i = 0; i < 100; i++) {
    pingCode = tclient.pingServer();
  }
  EXPECT_TRUE(tclient.waitForPong(pingCode, 5));

  for (int i = 0; i < 100; i++) {
    pingCode = tclient.pingServer();
    EXPECT_TRUE(tclient.waitForPong(pingCode, 5));
  }

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, Connection) {
  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  //  tclient.setVerbose(true);
  EXPECT_TRUE(tclient.start());

  EXPECT_EQ(tserver.connectionCount(), 1);
  EXPECT_TRUE(tclient.isConnected());

  tclient.stop();
  tserver.stop();
}
