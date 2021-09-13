#ifndef TINCSERVER_HPP
#define TINCSERVER_HPP

/*
 * Copyright 2020 AlloSphere Research Group
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 *        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * authors: Andres Cabrera, Kon Hyong Kim
 */

#include <deque>
#include <map>

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

  // See documentation on TincProtocol
  bool sendTincMessage(void *msg, al::Socket *dst = nullptr,
                       al::ValueSource *src = nullptr) override;

  void setVerbose(bool verbose);
  bool verbose() { return TincProtocol::mVerbose; }

  /**
   * @brief Initiate a barrier by notifying all connections to lock, and then
   * unlock after all acknowledge
   * @param group group to make the barrier for. 0 is all.
   * @param timeoutsec timeout in seconds
   * @return true if all connections locked and unlock message was sent before
   * the timeout
   *
   * This requires both server and clients to explicitly call barrier(). This
   * is used to syncrhonize applications so they resume execution after all have
   * finished executing a piece of code.
   */
  bool barrier(uint32_t group = 0, float timeoutsec = 0.0) override;

  std::pair<std::string, uint16_t> serverAddress();

  void disconnectAllClients();

  void markBusy() override;
  void markAvailable() override;

protected:
  void onConnection(al::Socket *newConnection) override;

  void processBarrierAckLock(al::Socket *src, uint64_t barrierConsecutive);
  void processClientMetadataMessage(void *details, al::Socket *src);
  void disconnectClient(al::Socket *src);

private:
  uint64_t mBarrierConsecutive{1};
  std::mutex mBarrierLock;

  // Network barriers
  std::map<uint64_t, std::deque<al::Socket *>> mBarrierAcks;
  std::mutex mBarrierAckLock;
};

} // namespace tinc

#endif // TINCSERVER_HPP
