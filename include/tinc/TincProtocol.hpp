#ifndef TINCPROTOCOL_HPP
#define TINCPROTOCOL_HPP

#include "al/io/al_Socket.hpp"
#include "al/protocol/al_CommandConnection.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/DataPool.hpp"
#include "tinc/DiskBuffer.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"

namespace tinc {

class TincProtocol {
public:
  // Data pool commands
  enum { CREATE_DATA_SLICE = 0x01 };

  // register parameter to tinc
  void registerParameter(al::ParameterMeta &param);
  void registerParameterSpace(ParameterSpace &ps);
  void registerParameterSpaceDimension(ParameterSpaceDimension &psd);
  void registerProcessor(Processor &processor);
  void registerDiskBuffer(AbstractDiskBuffer &db);
  void registerDataPool(DataPool &dp);
  void registerParameterServer(al::ParameterServer &pserver);

  TincProtocol &operator<<(al::ParameterMeta &p);
  TincProtocol &operator<<(ParameterSpace &p);
  TincProtocol &operator<<(ParameterSpaceDimension &psd);
  TincProtocol &operator<<(Processor &p);
  TincProtocol &operator<<(AbstractDiskBuffer &db);
  TincProtocol &operator<<(DataPool &dp);
  TincProtocol &operator<<(al::ParameterServer &pserver);

  // Send requests for data
  void requestParameters(al::Socket *dst);
  void requestParameterSpaces(al::Socket *dst);
  void requestProcessors(al::Socket *dst);
  void requestDiskBuffers(al::Socket *dst);
  void requestDataPools(al::Socket *dst);

  al::ParameterMeta *getParameter(std::string name) {
    for (auto *param : mParameters) {
      if (param->getName() == name) {
        return param;
      }
    }
    return nullptr;
  }

  std::vector<ParameterSpaceDimension *> dimensions() {
    // TODO protect possible race conditions.
    return mParameterSpaceDimensions;
  }

  void setVerbose(bool v) { mVerbose = v; }

protected:
  // Incoming request message
  void runRequest(int objectType, std::string objectId, al::Socket *src);
  // Outgoing messages
  void sendParameters(al::Socket *dst);
  void sendParameterSpace(al::Socket *dst);
  void sendProcessors(al::Socket *dst);
  void sendDataPools(al::Socket *dst);
  void sendDiskBuffers(al::Socket *dst);

  // Outgoing response message to requests
  void sendRequestResponse(al::ParameterMeta *param, al::Socket *dst);

  // Incoming register message
  bool runRegister(int objectType, void *any, al::Socket *src);
  bool processRegisterParameter(void *any, al::Socket *src);
  bool processRegisterParameterSpace(al::Message &message, al::Socket *src);
  bool processRegisterProcessor(al::Message &message, al::Socket *src);
  bool processRegisterDataPool(al::Message &message, al::Socket *src);
  bool processRegisterDiskBuffer(void *any, al::Socket *src);

  // Outgoing register message
  void sendRegisterParameterMessage(al::ParameterMeta *param, al::Socket *dst);
  void sendRegisterParameterSpaceMessage(ParameterSpace *ps, al::Socket *dst);
  void sendRegisterParameterSpaceDimensionMessage(ParameterSpaceDimension *dim,
                                                  al::Socket *dst);
  void sendRegisterProcessorMessage(Processor *p, al::Socket *dst);
  void sendRegisterDataPoolMessage(DataPool *p, al::Socket *dst);
  void sendRegisterDiskBufferMessage(AbstractDiskBuffer *p, al::Socket *dst);

  // Incoming configure message
  bool runConfigure(int objectType, void *any, al::Socket *src);
  bool processConfigureParameter(void *any, al::Socket *src);
  bool processConfigureParameterSpace(al::Message &message, al::Socket *src);
  bool processConfigureProcessor(al::Message &message, al::Socket *src);
  bool processConfigureDataPool(al::Message &message, al::Socket *src);
  bool processConfigureDiskBuffer(void *any, al::Socket *src);

  // Outgoing configure message (value + details)
  void sendParameterFloatDetails(al::Parameter *param, al::Socket *dst);
  void sendParameterIntDetails(al::ParameterInt *param, al::Socket *dst);
  void sendParameterStringDetails(al::ParameterString *param, al::Socket *dst);
  void sendParameterChoiceDetails(al::ParameterChoice *param, al::Socket *dst);
  void sendParameterColorDetails(al::ParameterColor *param, al::Socket *dst);
  void sendParameterSpaceMessage(ParameterSpaceDimension *dim, al::Socket *dst);
  void sendConfigureProcessorMessage(Processor *p, al::Socket *dst);
  void sendConfigureDataPoolMessage(DataPool *p, al::Socket *dst);

  // Outgoing configure message (only value) for callback functions
  void sendParameterFloatValue(float value, std::string fullAddress,
                               al::ValueSource *src);
  void sendParameterIntValue(int32_t value, std::string fullAddress,
                             al::ValueSource *src);
  void sendParameterUint64Value(uint64_t value, std::string fullAddress,
                                al::ValueSource *src);
  void sendParameterStringValue(std::string value, std::string fullAddress,
                                al::ValueSource *src);
  void sendParameterColorValue(al::Color value, std::string fullAddress,
                               al::ValueSource *src);

  // Incoming command message
  bool runCommand(int objectType, void *any, al::Socket *src);
  bool processCommandParameter(void *any, al::Socket *src);
  bool processCommandParameterSpace(void *any, al::Socket *src);
  bool processCommandDataPool(void *any, al::Socket *src);

  // send proto message (No checks. sends to dst socket)
  bool sendProtobufMessage(void *message, al::Socket *dst);

  // Send tinc message. Overriden on TincServer or TincClient
  virtual bool sendTincMessage(void *msg, al::Socket *dst = nullptr,
                               al::ValueSource *src = nullptr) {
    std::cerr << "Using unimplemented virtual function" << std::endl;
    return true;
  }

  std::vector<al::ParameterMeta *> mParameters;
  std::vector<ParameterSpace *> mParameterSpaces;
  std::vector<ParameterSpaceDimension *> mParameterSpaceDimensions;
  std::vector<Processor *> mProcessors;
  std::vector<AbstractDiskBuffer *> mDiskBuffers;
  std::vector<DataPool *> mDataPools;
  std::vector<al::ParameterServer *> mParameterServers;

  bool mVerbose{false};
};
} // namespace tinc
#endif // TINCPROTOCOL_HPP