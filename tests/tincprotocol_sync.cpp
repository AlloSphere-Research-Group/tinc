#include "gtest/gtest.h"

#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"
#include "tinc/CppProcessor.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(TincProtocol, Sync) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.1);
  ParameterSpaceDimension dim{"dim"};
  CppProcessor proc{"proc"};

  uint8_t sharedValue = 0;

  proc.processingFunction = [&]() {
    auto value = proc.configuration["dim"];
    std::this_thread::sleep_for(std::chrono::seconds(3));
    sharedValue = 1 * value.valueDouble;
    return true;
  };

  proc.registerDimension(dim);

  tclient.

      std::thread th_server([&]() { EXPECT_TRUE(tserver.barrier()); });
  al::al_sleep(0.1);
  std::thread th_clients([&]() { EXPECT_TRUE(tclient.barrier()); });

  th_server.join();
  th_clients.join();

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, ClientFirst) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.5);
  std::thread th_clients([&]() { EXPECT_TRUE(tclient.barrier()); });
  al::al_sleep(0.1);
  std::thread th_server([&]() { EXPECT_TRUE(tserver.barrier()); });

  th_server.join();
  th_clients.join();
  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, FastTrigger) {
  TincServer tserver;
  EXPECT_TRUE(tserver.start());

  TincClient tclient;
  EXPECT_TRUE(tclient.start());

  al::al_sleep(0.5);

  //   Multiple quick ones
  for (int i = 0; i < 30; i++) {
    std::thread th_clients2([&]() { EXPECT_TRUE(tclient.barrier()); });
    std::thread th_server2([&]() { EXPECT_TRUE(tserver.barrier()); });

    th_server2.join();
    th_clients2.join();
  }

  tclient.stop();
  tserver.stop();
}

TEST(TincProtocol, BarrierMultiClient) {
  // FIXME fix support for multiclient barrier
  //  TincServer tserver;
  //  EXPECT_TRUE(tserver.start());

  //  TincClient tclient;
  //  EXPECT_TRUE(tclient.start());

  //  TincClient tclient2;
  //  EXPECT_TRUE(tclient2.start());

  //  TincClient tclient3;
  //  EXPECT_TRUE(tclient3.start());

  //  TincClient tclient4;
  //  EXPECT_TRUE(tclient4.start());

  //  al::al_sleep(1.0);

  //  //  std::thread th_clients([&]() {
  //  //    tclient.barrier();
  //  //    tclient2.barrier();
  //  //  });
  //  std::thread th_server([&]() { tserver.barrier(); });
  //  std::thread th_clients2([&]() {
  //    tclient3.barrier();
  //    tclient4.barrier();
  //  });

  //  th_server.join();
  //  //  th_clients.join();
  //  th_clients2.join();

  //  tclient.stop();
  //  tclient2.stop();
  //  tclient3.stop();
  //  tclient4.stop();
  //  tserver.stop();
}
