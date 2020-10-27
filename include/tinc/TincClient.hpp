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

  inline void sendParameters() { TincProtocol::sendParameters(&mSocket); }

  inline void requestParameters() { TincProtocol::requestParameters(&mSocket); }
  inline void requestProcessors() { TincProtocol::requestProcessors(&mSocket); }
  inline void requestDiskBuffers() {
    TincProtocol::requestDiskBuffers(&mSocket);
  }
  inline void requestDataPools() { TincProtocol::requestDataPools(&mSocket); }
  inline void requestParameterSpaces() {
    TincProtocol::requestParameterSpaces(&mSocket);
  }

  // determine if message needs to be propagated
  bool shouldSendMessage(al::Socket *dst) override;

  void setVerbose(bool verbose);
  bool verbose() { return TincProtocol::mVerbose; }

private:
};

} // namespace tinc

#endif // TINCCLIENT_HPP
