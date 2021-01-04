#ifndef TINCPROTOCOL_HPP
#define TINCPROTOCOL_HPP

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

#include "al/io/al_Socket.hpp"
#include "al/protocol/al_CommandConnection.hpp"

#include "tinc/DataPool.hpp"
#include "tinc/DiskBuffer.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"

namespace tinc {

class TincProtocol {
public:
  // Data pool commands
  enum { CREATE_DATA_SLICE = 0x01 };

  typedef enum {
    STATUS_UNKNOWN = 0x00,
    STATUS_AVAILABLE = 0x01,
    STATUS_BUSY = 0x02
  } Status;

  /**
   * @brief register a parameter with this Tinc node
   * @param param the parameter to register
   * @param src the source socket of the request
   *
   * If src is not nullptr, network notification of registration will be blocked
   * for that destination.
   */
  void registerParameter(al::ParameterMeta &param, al::Socket *src = nullptr);

  /**
   * @brief register a parameter space dimencion with this Tinc node
   * @param ps the parameter space to register
   * @param src the source socket of the request
   *
   * If src is not nullptr, network notification of registration will be blocked
   * for that destination.
   * This is equivalent to registerParameterSpaceDimension() and will internally
   * wrap the parameter in a dimension.
   */
  void registerParameterSpaceDimension(ParameterSpaceDimension &psd,
                                       al::Socket *src = nullptr);

  /**
   * @brief register a parameter space with this Tinc node
   * @param psd the parameter space dimension to register
   * @param src the source socket of the request
   *
   * If src is not nullptr, network notification of registration will be blocked
   * for that destination.
   */
  void registerParameterSpace(ParameterSpace &ps, al::Socket *src = nullptr);

  /**
   * @brief register a processor with this Tinc node
   * @param processor the processor to register
   * @param src the source socket of the request
   *
   * If src is not nullptr, network notification of registration will be blocked
   * for that destination.
   */
  void registerProcessor(Processor &processor, al::Socket *src = nullptr);

  /**
   * @brief register a disk buffer with this Tinc node
   * @param db the disk buffer to register
   * @param src the source socket of the request
   *
   * If src is not nullptr, network notification of registration will be blocked
   * for that destination.
   */
  void registerDiskBuffer(DiskBufferAbstract &db, al::Socket *src = nullptr);

  /**
   * @brief register a data pool with this Tinc node
   * @param dp the data pool to register
   * @param src the source socket of the request
   *
   * If src is not nullptr, network notification of registration will be blocked
   * for that destination.
   */
  void registerDataPool(DataPool &dp, al::Socket *src = nullptr);

  TincProtocol &operator<<(al::ParameterMeta &p);
  TincProtocol &operator<<(ParameterSpace &p);
  TincProtocol &operator<<(ParameterSpaceDimension &psd);
  TincProtocol &operator<<(Processor &p);
  TincProtocol &operator<<(DiskBufferAbstract &db);
  TincProtocol &operator<<(DataPool &dp);

  // Send requests for data
  // FIXME make request automatic with start, or make a requestall?
  void requestParameters(al::Socket *dst);
  void requestParameterSpaces(al::Socket *dst);
  void requestProcessors(al::Socket *dst);
  void requestDiskBuffers(al::Socket *dst);
  void requestDataPools(al::Socket *dst);

  /**
   * @brief get a parameter from a registered dimension in this Tinc node
   * @param name name (id) of the parameter
   * @param group
   * @return a pointer to the parameter or nullptr if not found
   *
   * If group is not provided, the osc address for the parameter will be matched
   * to name.
   */
  al::ParameterMeta *getParameter(std::string name, std::string group = "");

  std::vector<ParameterSpaceDimension *> dimensions() {
    // TODO protect possible race conditions.
    return mParameterSpaceDimensions;
  }

  std::vector<ParameterSpace *> parameterSpaces() {
    // TODO protect possible race conditions.
    return mParameterSpaces;
  }

  /**
   * @brief activate a network barrier
   * @param group group to make the barrier for. 0 is all.
   * @param timeoutsec timeout in secods
   * @return true if barrier succeeded before timing out
   *
   * Group support not implemented yet.
   */
  virtual bool barrier(uint32_t group = 0, float timeoutsec = 0.0) = 0;

  void setVerbose(bool v) { mVerbose = v; }

  virtual void markBusy();

  virtual void markAvailable();

protected:
  void connectParameterCallbacks(al::ParameterMeta &param);
  void connectDimensionCallbacks(ParameterSpaceDimension &psd);
  // Incoming request message
  void readRequestMessage(int objectType, std::string objectId,
                          al::Socket *src);
  void processRequestParameters(al::Socket *dst);
  void processRequestParameterSpaces(al::Socket *dst);
  void processRequestProcessors(al::Socket *dst);
  void processRequestDataPools(al::Socket *dst);
  void processRequestDiskBuffers(al::Socket *dst);

  // Incoming register message
  bool readRegisterMessage(int objectType, void *any, al::Socket *src);
  bool processRegisterParameter(void *any, al::Socket *src);
  bool processRegisterParameterSpace(void *any, al::Socket *src);
  bool processRegisterProcessor(void *any, al::Socket *src);
  bool processRegisterDataPool(void *any, al::Socket *src);
  bool processRegisterDiskBuffer(void *any, al::Socket *src);

  // Outgoing register message
  void sendRegisterMessage(ParameterSpaceDimension *dim, al::Socket *dst,
                           al::Socket *src = nullptr);
  void sendRegisterMessage(ParameterSpace *ps, al::Socket *dst,
                           al::Socket *src = nullptr);
  void sendRegisterMessage(Processor *p, al::Socket *dst,
                           al::Socket *src = nullptr);
  void sendRegisterMessage(DataPool *p, al::Socket *dst,
                           al::Socket *src = nullptr);
  void sendRegisterMessage(DiskBufferAbstract *p, al::Socket *dst,
                           al::Socket *src = nullptr);

  // Incoming configure message
  bool readConfigureMessage(int objectType, void *any, al::Socket *src);
  bool processConfigureParameter(void *any, al::Socket *src);
  bool processConfigureParameterSpace(void *any, al::Socket *src);
  bool processConfigureProcessor(void *any, al::Socket *src);
  bool processConfigureDataPool(void *any, al::Socket *src);
  bool processConfigureDiskBuffer(void *any, al::Socket *src);

  // Outgoing configure message (value + details)
  void sendConfigureMessage(ParameterSpaceDimension *dim, al::Socket *dst,
                            al::Socket *src = nullptr);
  void sendConfigureMessage(ParameterSpace *ps, al::Socket *dst,
                            al::Socket *src = nullptr);
  void sendConfigureMessage(Processor *p, al::Socket *dst,
                            al::Socket *src = nullptr);
  void sendConfigureMessage(DataPool *p, al::Socket *dst,
                            al::Socket *src = nullptr);
  void sendConfigureMessage(DiskBufferAbstract *p, al::Socket *dst,
                            al::Socket *src = nullptr);

  void sendConfigureParameterSpaceAddDimension(ParameterSpace *ps,
                                               ParameterSpaceDimension *dim,
                                               al::Socket *dst,
                                               al::Socket *src = nullptr);
  void sendConfigureParameterSpaceRemoveDimension(ParameterSpace *ps,
                                                  ParameterSpaceDimension *dim,
                                                  al::Socket *dst,
                                                  al::Socket *src = nullptr);

  // Outgoing configure message (only value) for callback functions
  void sendValueMessage(float value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(bool value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(int32_t value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(uint64_t value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(std::string value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(al::Vec3f value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(al::Vec4f value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(al::Color value, std::string fullAddress,
                        al::ValueSource *src = nullptr);
  void sendValueMessage(al::Pose value, std::string fullAddress,
                        al::ValueSource *src = nullptr);

  // Incoming command message
  bool readCommandMessage(int objectType, void *any, al::Socket *src);
  bool sendCommandErrorMessage(uint64_t commandId, std::string objectId,
                               std::string errorMessage, al::Socket *src);

  bool processCommandParameter(void *any, al::Socket *src);
  bool processCommandParameterSpace(void *any, al::Socket *src);
  bool processCommandDataPool(void *any, al::Socket *src);

  // send proto message (No checks. sends to dst socket)
  bool sendProtobufMessage(void *message, al::Socket *dst);

  // Send tinc message. Overriden on TincServer or TincClient
  /**
   * @brief sendTincMessage
   * @param msg The encoded message to send
   * @param dst the socket to send meessage to. If nullptr send to all
   * @param src information of source. If passed block sending to this location.
   * @return true on succesful send.
   */
  virtual bool sendTincMessage(void *msg, al::Socket *dst = nullptr,
                               al::ValueSource *src = nullptr) {
    std::cerr << __FUNCTION__ << ": Using invalid virtual implementation"
              << std::endl;
    return true;
  }

  std::vector<ParameterSpace *> mParameterSpaces;
  std::vector<ParameterSpaceDimension *> mParameterSpaceDimensions;
  std::vector<Processor *> mProcessors;
  std::vector<DiskBufferAbstract *> mDiskBuffers;
  std::vector<DataPool *> mDataPools;

  // Dimensions that were allocated by this class
  std::vector<std::unique_ptr<ParameterSpaceDimension>> mLocalPSDs;

  // Barriers
  int barrierWaitGranularTimeMs = 20;

  std::mutex mBusyCountLock;
  uint32_t mBusyCount = 0;

  bool mVerbose{false};
};
} // namespace tinc
#endif // TINCPROTOCOL_HPP
