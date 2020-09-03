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
  enum {
    // Requests from client
    REQUEST_PARAMETERS = 0x21,
    REQUEST_PROCESSORS = 0x22,
    REQUEST_DISK_BUFFERS = 0x23,
    REQUEST_DATA_POOLS = 0x24,
    REQUEST_PARAMETER_SPACES = 0x25,

    // Server messages
    REGISTER_PARAMETER = 0x41,
    REGISTER_PROCESSOR = 0x42,
    REGISTER_DISK_BUFFER = 0x43,
    REGISTER_DATA_POOL = 0x44,
    REGISTER_PARAMETER_SPACE = 0x45,

    REMOVE_PARAMETER = 0x61,
    REMOVE_PROCESSOR = 0x62,
    REMOVE_DISK_BUFFER = 0x63,
    REMOVE_DATA_POOL = 0x64,
    REMOVE_PARAMETER_SPACE = 0x65,

    CONFIGURE_PARAMETER = 0x81,
    CONFIGURE_PROCESSOR = 0x82,
    CONFIGURE_DISK_BUFFER = 0x83,
    CONFIGURE_DATA_POOL = 0x84,
    CONFIGURE_PARAMETER_SPACE = 0x84,

    OBJECT_COMMAND = 0xA0,
    OBJECT_COMMAND_REPLY = 0xA1,
    OBJECT_COMMAND_ERROR = 0xA2,
  };

  enum {
    PARAMETER = 0x01,
    PARAMETER_STRING = 0x02,
    PARAMETER_INT = 0x03,
    PARAMETER_VEC3 = 0x04,
    PARAMETER_COLOR = 0x05
  };

  enum { DATA_DOUBLE = 0x01, DATA_INT64 = 0x02, DATA_STRING = 0x03 };

  enum {
    PROCESSOR_CPP = 0x01,
    PROCESSOR_DATASCRIPT = 0x02,
    PROCESSOR_CHAIN = 0x03,
    PROCESSOR_UNKNOWN = 0xFF
  };

  // Data pool commands
  enum { CREATE_DATA_SLICE = 0x01 };

  static_assert(REQUEST_PARAMETERS >= COMMAND_LAST_INTERNAL, "Command overlap");

  TincServer();

  void registerParameterServer(al::ParameterServer &pserver) {
    mParameterServers.push_back(&pserver);
  }

  void registerProcessor(Processor &processor) {
    mProcessors.push_back(&processor);
  }

  void registerParameterSpace(ParameterSpace &ps) {
    mParameterSpaces.push_back(&ps);
  }

  void registerDiskBuffer(AbstractDiskBuffer &db) {
    mDiskBuffers.push_back(&db);
  }

  void registerDataPool(DataPool &dp) { mDataPools.push_back(&dp); }

  TincServer &operator<<(Processor &p) {
    registerProcessor(p);
    return *this;
  }

  TincServer &operator<<(ParameterSpace &p) {
    registerParameterSpace(p);
    return *this;
  }

  TincServer &operator<<(AbstractDiskBuffer &db) {
    registerDiskBuffer(db);
    return *this;
  }

  TincServer &operator<<(DataPool &db) {
    registerDataPool(db);
    return *this;
  }

  bool processIncomingMessage(uint8_t *message, size_t length,
                              al::Socket *src) override;

protected:
  std::vector<Processor *> mProcessors;
  std::vector<al::ParameterServer *> mParameterServers;
  std::vector<ParameterSpace *> mParameterSpaces;
  std::vector<AbstractDiskBuffer *> mDiskBuffers;
  std::vector<DataPool *> mDataPools;

private:
  void sendParameters();
  void sendProcessors();
  void sendDataPools();
  void sendRegisterProcessorMessage(Processor *p);
  void sendConfigureProcessorMessage(Processor *p);
  void sendRegisterDataPoolMessage(DataPool *p);
  void sendRegisterParameterSpaceMessage(ParameterSpace *p);

  bool processObjectCommand(uint8_t *message, size_t length, al::Socket *src);
};

} // namespace tinc

#endif // TINCSERVER_HPP
