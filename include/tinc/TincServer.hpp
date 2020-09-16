#ifndef TINCSERVER_HPP
#define TINCSERVER_HPP

#include "al/protocol/al_CommandConnection.hpp"
#include "al/io/al_Socket.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/DiskBuffer.hpp"
#include "tinc/DataPool.hpp"
#include "tinc/ParameterSpace.hpp"
#include "tinc/Processor.hpp"

namespace tinc {

class TincServer : public al::CommandServer {
public:
  // Data pool commands
  enum { CREATE_DATA_SLICE = 0x01 };

  TincServer();

  void registerParameterServer(al::ParameterServer &pserver);
  void registerProcessor(Processor &processor);
  void registerParameterSpace(ParameterSpace &ps);
  void registerParameter(al::ParameterMeta &p);
  void registerParameterSpaceDimension(ParameterSpaceDimension &psd);
  void registerDiskBuffer(AbstractDiskBuffer &db);
  void registerDataPool(DataPool &dp);

  TincServer &operator<<(Processor &p);
  TincServer &operator<<(ParameterSpace &p);
  TincServer &operator<<(AbstractDiskBuffer &db);
  TincServer &operator<<(DataPool &db);

  bool processIncomingMessage(al::Message &message, al::Socket *src) override;

protected:
  std::vector<Processor *> mProcessors;
  std::vector<al::ParameterServer *> mParameterServers;
  std::vector<ParameterSpace *> mParameterSpaces;
  std::vector<ParameterSpaceDimension *> mParameterSpaceDimensions;
  std::vector<AbstractDiskBuffer *> mDiskBuffers;
  std::vector<DataPool *> mDataPools;

private:
  void runRequest(int objectType, std::string objectId, al::Socket *src);

  void sendParameters(al::Socket *dst);
  void sendParameterSpace(al::Socket *dst);
  void sendProcessors(al::Socket *dst);
  void sendDataPools(al::Socket *dst);
  void sendDiskBuffers(al::Socket *dst);
  void sendRegisterProcessorMessage(Processor *p, al::Socket *dst);
  void sendConfigureProcessorMessage(Processor *p, al::Socket *dst);
  void sendRegisterDataPoolMessage(DataPool *p, al::Socket *dst);
  //  void sendRegisterDiskBufferlMessage(DiskBuffer *p, al::Socket *dst);
  void sendRegisterParameterSpaceMessage(ParameterSpace *p, al::Socket *dst);
  void sendRegisterParameterSpaceDimensionMessage(ParameterSpaceDimension *dim,
                                                  al::Socket *dst);
  void sendParameterFloatDetails(al::Parameter *param, al::Socket *dst);
  void sendParameterIntDetails(al::ParameterInt *param, al::Socket *dst);
  void sendRegisterParameterMessage(al::ParameterMeta *param, al::Socket *dst);

  bool processObjectCommand(al::Message &message, al::Socket *src);

  bool runConfigure(int objectType, void *any, al::Socket *src);
  bool processConfigureParameter(void *any, al::Socket *src);
  bool processConfigureParameterSpace(al::Message &message, al::Socket *src);
  bool processConfigureProcessor(al::Message &message, al::Socket *src);
  bool processConfigureDataPool(al::Message &message, al::Socket *src);
  bool processConfigureDiskBuffer(al::Message &message, al::Socket *src);
};

} // namespace tinc

#endif // TINCSERVER_HPP
