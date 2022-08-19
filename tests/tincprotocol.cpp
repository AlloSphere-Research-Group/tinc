#include "gtest/gtest.h"

#include "tinc/DiskBufferJson.hpp"
#include "tinc/TincClient.hpp"
#include "tinc/TincServer.hpp"

#include "al/system/al_Time.hpp"
using namespace tinc;

TEST(TincProtocol, MultiConnectionDiskBuffer) {

  TincServer tserver;
  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  std::vector<TincClient *> clients;
  int numConnections = 15;
  for (int i = 0; i < numConnections; i++) {
    TincClient *tclient = new TincClient;
    clients.push_back(tclient);
    tclient->setVerbose(true);
    tclient->start();
  }

  tserver.waitForConnections(numConnections, 2.0);
  EXPECT_EQ(tserver.connectionCount(), numConnections);

  DiskBufferJson buf{"name", "file.json"};
  tserver.registerDiskBuffer(buf);

  al::al_sleep(0.5);

  for (int i = 0; i < numConnections; i++) {
    EXPECT_TRUE(clients[i]->getDiskBuffer("name"));
  }

  auto data = nlohmann::basic_json<>{{"hello", 42}};
  buf.setData(data);

  al::al_sleep(0.5);
  for (int i = 0; i < numConnections; i++) {
    auto clientBuf =
        static_cast<DiskBufferJson *>(clients[i]->getDiskBuffer("name"));

    EXPECT_TRUE(clientBuf->newDataAvailable());
    std::shared_ptr<nlohmann::basic_json<>> data = clientBuf->get();
    EXPECT_TRUE(data->is_object());
    EXPECT_EQ(data->at("hello"), 42);
  }

  for (int i = 0; i < numConnections; i++) {
    clients[i]->stop();
  }

  al::al_sleep(0.5);
  EXPECT_EQ(tserver.connectionCount(), 0);

  tserver.stop();
}

TEST(TincProtocol, MultiConnectionParameter) {

  TincServer tserver;
  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  std::vector<TincClient *> clients;
  int numConnections = 15;
  for (int i = 0; i < numConnections; i++) {
    TincClient *tclient = new TincClient;
    clients.push_back(tclient);
    tclient->setVerbose(true);
    tclient->start();
  }

  tserver.waitForConnections(numConnections, 2.0);
  EXPECT_EQ(tserver.connectionCount(), numConnections);

  ParameterSpaceDimension dim{"name"};
  tserver.registerParameterSpaceDimension(dim);

  al::al_sleep(0.1);

  for (int i = 0; i < numConnections; i++) {
    EXPECT_EQ(clients[i]->dimensions().size(), 1);
  }
  dim.setCurrentValue(0.1);

  al::al_sleep(0.1);
  for (int i = 0; i < numConnections; i++) {
    EXPECT_FLOAT_EQ(clients[i]->dimensions()[0]->getCurrentValue(), 0.1);
  }
  dim.setCurrentValue(0.2);

  al::al_sleep(0.1);
  for (int i = 0; i < numConnections; i++) {
    EXPECT_FLOAT_EQ(clients[i]->dimensions()[0]->getCurrentValue(), 0.2);
  }

  for (int i = 0; i < numConnections; i++) {
    clients[i]->stop();
  }

  al::al_sleep(0.1);
  // FIXME have a way to wait until all connections are closed
  //  EXPECT_EQ(tserver.connectionCount(), 0);

  tserver.stop();
}

TEST(TincProtocol, MultiConnectionMany) {

  TincServer tserver;
  //  tserver.setVerbose(true);
  EXPECT_TRUE(tserver.start());

  std::vector<TincClient *> clients;
  int numConnections = 20; // Larger numbers have issues
  for (int i = 0; i < numConnections; i++) {
    TincClient *tclient = new TincClient;
    clients.push_back(tclient);
    //    tclient->setVerbose(true);
    tclient->start();
  }

  tserver.waitForConnections(numConnections, 2.0);
  EXPECT_EQ(tserver.connectionCount(), numConnections);

  for (int i = 0; i < numConnections; i++) {
    clients[i]->stop();
    al::al_sleep(0.1);
  }
  al::al_sleep(0.2);
  // FIXME have a way to wait until all connections are closed
  //  EXPECT_EQ(tserver.connectionCount(), 0);

  tserver.stop();
}

TEST(TincProtocol, MultiConnectionStress) {
  // FIXME this fails
  //  for (int i = 0; i < 20; i++) {
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

  //    tserver.waitForConnections(4, 2.0);

  //    EXPECT_EQ(tserver.connectionCount(), 4);
  //    EXPECT_TRUE(tclient.isConnected());
  //    EXPECT_TRUE(tclient2.isConnected());
  //    EXPECT_TRUE(tclient3.isConnected());
  //    EXPECT_TRUE(tclient4.isConnected());

  //    tclient.stop();
  //    tserver.waitForConnections(3, 2.0);

  //    EXPECT_EQ(tserver.connectionCount(), 3);
  //    EXPECT_TRUE(!tclient.isConnected());
  //    EXPECT_TRUE(tclient2.isConnected());
  //    EXPECT_TRUE(tclient3.isConnected());
  //    EXPECT_TRUE(tclient4.isConnected());

  //    tclient2.stop();
  //    tserver.waitForConnections(2, 2.0);

  //    EXPECT_EQ(tserver.connectionCount(), 2);
  //    EXPECT_TRUE(!tclient.isConnected());
  //    EXPECT_TRUE(!tclient2.isConnected());
  //    EXPECT_TRUE(tclient3.isConnected());
  //    EXPECT_TRUE(tclient4.isConnected());

  //    tclient3.stop();
  //    tserver.waitForConnections(1, 2.0);

  //    EXPECT_EQ(tserver.connectionCount(), 1);
  //    EXPECT_TRUE(!tclient.isConnected());
  //    EXPECT_TRUE(!tclient2.isConnected());
  //    EXPECT_TRUE(!tclient3.isConnected());
  //    EXPECT_TRUE(tclient4.isConnected());

  //    tclient4.stop();
  //    tserver.waitForConnections(0, 2.0);

  //    EXPECT_EQ(tserver.connectionCount(), 0);
  //    EXPECT_TRUE(!tclient.isConnected());
  //    EXPECT_TRUE(!tclient2.isConnected());
  //    EXPECT_TRUE(!tclient3.isConnected());
  //    EXPECT_TRUE(!tclient4.isConnected());

  //    tserver.stop();
  //    al::al_sleep(1.0);
  //  }
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
