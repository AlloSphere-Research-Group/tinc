#ifndef TINCCLIENT_HPP
#define TINCCLIENT_HPP

#include "al/io/al_Socket.hpp"
#include "al/protocol/al_CommandConnection.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/DataPool.hpp"
#include "tinc/DiskBuffer.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"
#include "tinc/TincProtocol.hpp"

namespace tinc {

class TincClient : public al::CommandClient, public TincProtocol {
public:
  TincClient();

  bool processIncomingMessage(al::Message &message, al::Socket *src) override;

  void sendTincMessage(void *msg, al::ValueSource *src = nullptr) override;

  void requestParameters() { TincProtocol::requestParameters(&mSocket); }
  void requestProcessors() { TincProtocol::requestProcessors(&mSocket); }
  void requestDiskBuffers() { TincProtocol::requestDiskBuffers(&mSocket); }
  void requestDataPools() { TincProtocol::requestDataPools(&mSocket); }
  void requestParameterSpaces() {
    TincProtocol::requestParameterSpaces(&mSocket);
  }

private:
};

} // namespace tinc

#endif // TINCCLIENT_HPP
