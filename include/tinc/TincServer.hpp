#ifndef TINCSERVER_HPP
#define TINCSERVER_HPP

#include "al/io/al_Socket.hpp"
#include "al/protocol/al_CommandConnection.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/DataPool.hpp"
#include "tinc/DiskBuffer.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"
#include "tinc/TincProtocol.hpp"

namespace tinc {

class TincServer : public al::CommandServer, public TincProtocol {
public:
  // Data pool commands
  enum { CREATE_DATA_SLICE = 0x01 };

  TincServer();

  bool processIncomingMessage(al::Message &message, al::Socket *src) override;

  bool sendTincMessage(void *msg, al::Socket *dst = nullptr,
                       bool isResponse = false,
                       al::ValueSource *src = nullptr) override;

  void setVerbose(bool verbose);
  bool verbose() { return TincProtocol::mVerbose; }

private:
};

} // namespace tinc

#endif // TINCSERVER_HPP
