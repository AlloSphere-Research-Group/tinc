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
  void registerParameter(al::ParameterMeta &param, al::Socket *dst = nullptr);
  void registerParameterSpace(ParameterSpace &ps, al::Socket *dst = nullptr);
  void registerParameterSpaceDimension(ParameterSpaceDimension &psd,
                                       al::Socket *dst = nullptr);
  void registerProcessor(Processor &processor, al::Socket *dst = nullptr);
  void registerDiskBuffer(AbstractDiskBuffer &db, al::Socket *dst = nullptr);
  void registerDataPool(DataPool &dp, al::Socket *dst = nullptr);
  // FIXME are parameterservers meant to be sent over network?
  void registerParameterServer(al::ParameterServer &pserver);

  TincProtocol &operator<<(al::ParameterMeta &p);
  TincProtocol &operator<<(ParameterSpace &p);
  TincProtocol &operator<<(ParameterSpaceDimension &psd);
  TincProtocol &operator<<(Processor &p);
  TincProtocol &operator<<(AbstractDiskBuffer &db);
  TincProtocol &operator<<(DataPool &dp);
  TincProtocol &operator<<(al::ParameterServer &pserver);

  // Send requests for data
  // FIXME make request automatic with start, or make a requestall?
  void requestParameters(al::Socket *dst);
  void requestParameterSpaces(al::Socket *dst);
  void requestProcessors(al::Socket *dst);
  void requestDiskBuffers(al::Socket *dst);
  void requestDataPools(al::Socket *dst);

  // TODO what if same name but different group?
  al::ParameterMeta *getParameter(std::string name, std::string group = "") {
    for (auto *dim : mParameterSpaceDimensions) {
      if (dim->getName() == name && dim->getGroup() == group) {
        return dim->parameterMeta();
      } else if (group == "" && dim->getFullAddress() == name) {
        return dim->parameterMeta();
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
  bool processRegisterParameterSpace(al::Message &message, al::Socket *src);
  bool processRegisterProcessor(al::Message &message, al::Socket *src);
  bool processRegisterDataPool(al::Message &message, al::Socket *src);
  bool processRegisterDiskBuffer(void *any, al::Socket *src);

  // Outgoing register message
  //  void sendRegisterMessage(al::ParameterMeta *param, al::Socket *dst,
  //                           bool isResponse = false);
  void sendRegisterMessage(ParameterSpace *ps, al::Socket *dst,
                           bool isResponse = false);
  void sendRegisterMessage(ParameterSpaceDimension *dim, al::Socket *dst,
                           bool isResponse = false);
  void sendRegisterMessage(Processor *p, al::Socket *dst,
                           bool isResponse = false);
  void sendRegisterMessage(DataPool *p, al::Socket *dst,
                           bool isResponse = false);
  void sendRegisterMessage(AbstractDiskBuffer *p, al::Socket *dst,
                           bool isResponse = false);

  // Incoming configure message
  bool readConfigureMessage(int objectType, void *any, al::Socket *src);
  bool processConfigureParameter(void *any, al::Socket *src);
  bool processConfigureParameterSpace(al::Message &message, al::Socket *src);
  bool processConfigureProcessor(al::Message &message, al::Socket *src);
  bool processConfigureDataPool(al::Message &message, al::Socket *src);
  bool processConfigureDiskBuffer(void *any, al::Socket *src);

  // Outgoing configure message (value + details)
  //  void sendConfigureMessage(al::ParameterMeta *param, al::Socket *dst,
  //                            bool isResponse = false);
  void sendConfigureMessage(ParameterSpace *ps, al::Socket *dst,
                            bool isResponse = false);
  void sendConfigureMessage(ParameterSpaceDimension *dim, al::Socket *dst,
                            bool isResponse = false);
  void sendConfigureMessage(Processor *p, al::Socket *dst,
                            bool isResponse = false);
  void sendConfigureMessage(DataPool *p, al::Socket *dst,
                            bool isResponse = false);
  void sendConfigureMessage(AbstractDiskBuffer *p, al::Socket *dst,
                            bool isResponse = false);

  // Outgoing configure message (only value) for callback functions
  void sendValueMessage(float value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(bool value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(int32_t value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(uint64_t value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(std::string value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(al::Vec3f value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(al::Vec4f value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(al::Color value, std::string fullAddress,
                        al::ValueSource *src);
  void sendValueMessage(al::Pose value, std::string fullAddress,
                        al::ValueSource *src);

  // Incoming command message
  bool readCommandMessage(int objectType, void *any, al::Socket *src);
  bool processCommandParameter(void *any, al::Socket *src);
  bool processCommandParameterSpace(void *any, al::Socket *src);
  bool processCommandDataPool(void *any, al::Socket *src);

  // send proto message (No checks. sends to dst socket)
  bool sendProtobufMessage(void *message, al::Socket *dst);

  // Send tinc message. Overriden on TincServer or TincClient
  virtual bool sendTincMessage(void *msg, al::Socket *dst = nullptr,
                               bool isResponse = false,
                               al::ValueSource *src = nullptr) {
    std::cerr << __FUNCTION__ << ": Using invalid virtual implementation"
              << std::endl;
    return true;
  }

  //  std::vector<al::ParameterMeta *> mParameters;
  std::vector<ParameterSpace *> mParameterSpaces;
  std::vector<ParameterSpaceDimension *> mParameterSpaceDimensions;
  std::vector<Processor *> mProcessors;
  std::vector<AbstractDiskBuffer *> mDiskBuffers;
  std::vector<DataPool *> mDataPools;
  std::vector<al::ParameterServer *> mParameterServers;

  // Dimensions that were allocated by this class to wrap a parameter
  std::vector<ParameterSpaceDimension *> mParameterWrappers;

  bool mVerbose{false};
};
} // namespace tinc
#endif // TINCPROTOCOL_HPP
