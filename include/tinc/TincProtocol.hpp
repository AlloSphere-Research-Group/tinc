#ifndef TINCPROTOCOL_HPP
#define TINCPROTOCOL_HPP

#include "al/protocol/al_CommandConnection.hpp"
#include "al/io/al_Socket.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/DiskBuffer.hpp"
#include "tinc/DataPool.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"

namespace tinc {

class TincProtocol {
public:
  // Data pool commands
  enum { CREATE_DATA_SLICE = 0x01 };

  void runRequest(int objectType, std::string objectId, al::Socket *src);

  void sendParameters(al::Socket *dst);
  void sendParameterSpace(al::Socket *dst);
  void sendProcessors(al::Socket *dst);
  void sendDataPools(al::Socket *dst);
  void sendDiskBuffers(al::Socket *dst);
  void sendRegisterProcessorMessage(Processor *p, al::Socket *dst);
  void sendConfigureProcessorMessage(Processor *p, al::Socket *dst);
  void sendRegisterDataPoolMessage(DataPool *p, al::Socket *dst);
  void sendRegisterDiskBufferMessage(AbstractDiskBuffer *p, al::Socket *dst);
  void sendRegisterParameterSpaceMessage(ParameterSpace *p, al::Socket *dst);
  void sendRegisterParameterSpaceDimensionMessage(ParameterSpaceDimension *dim,
                                                  al::Socket *dst);
  void sendParameterSpaceMessage(ParameterSpaceDimension *dim, al::Socket *dst);
  void sendParameterFloatDetails(al::Parameter *param, al::Socket *dst);
  void sendParameterIntDetails(al::ParameterInt *param, al::Socket *dst);
  void sendParameterStringDetails(al::ParameterString *param, al::Socket *dst);
  void sendParameterChoiceDetails(al::ParameterChoice *param, al::Socket *dst);
  void sendParameterColorDetails(al::ParameterColor *param, al::Socket *dst);
  void sendRegisterParameterMessage(al::ParameterMeta *param, al::Socket *dst);

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

  bool sendProtobufMessage(void *message, al::Socket *dst);

  bool processObjectCommand(al::Message &message, al::Socket *src);

  bool runConfigure(int objectType, void *any, al::Socket *src);
  bool processConfigureParameter(void *any, al::Socket *src);
  bool processConfigureParameterSpace(al::Message &message, al::Socket *src);
  bool processConfigureProcessor(al::Message &message, al::Socket *src);
  bool processConfigureDataPool(al::Message &message, al::Socket *src);
  bool processConfigureDiskBuffer(void *any, al::Socket *src);

  virtual void sendTincMessage(void *msg, al::ValueSource *src = nullptr) = 0;

  // Requests to server
  void requestParameters(al::Socket *dst = nullptr);
  void requestProcessors(al::Socket *dst = nullptr);
  void requestDiskBuffers(al::Socket *dst = nullptr);
  void requestDataPools(al::Socket *dst = nullptr);
  void requestParameterSpaces(al::Socket *dst = nullptr);

  al::ParameterMeta *getParameter(std::string name) {
    for (auto *param : mParameters) {
      if (param->getName() == name) {
        return param;
      }
    }
    return nullptr;
  }

protected:
  std::vector<al::ParameterMeta *> mParameters;
  std::vector<Processor *> mProcessors;
  std::vector<al::ParameterServer *> mParameterServers;
  std::vector<ParameterSpace *> mParameterSpaces;
  std::vector<ParameterSpaceDimension *> mParameterSpaceDimensions;
  std::vector<AbstractDiskBuffer *> mDiskBuffers;
  std::vector<DataPool *> mDataPools;
};

} // namespace tinc

#endif // TINCPROTOCOL_HPP
