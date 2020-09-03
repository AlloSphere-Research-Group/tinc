#ifndef COMPUTATIONCHAIN_HPP
#define COMPUTATIONCHAIN_HPP

#include "tinc/ProcessorAsync.hpp"
#include "tinc/ScriptProcessor.hpp"

#include <mutex>
#include <thread>

namespace tinc {

class ComputationChain : public Processor {
public:
  typedef enum { PROCESS_SERIAL, PROCESS_ASYNC } ChainType;
  ComputationChain(ChainType type = PROCESS_SERIAL, std::string id = "")
      : Processor(id), mType(type) {}
  ComputationChain(std::string id) : Processor(id), mType(PROCESS_SERIAL) {}

  void addProcessor(Processor &chain);

  bool process(bool forceRecompute = false) override;

  /**
   * @brief get map of results of computation by processor name
   *
   * This function will block if computation is currently executing and will
   * return after computation is done.
   */
  std::map<std::string, bool> getResults() {
    std::unique_lock<std::mutex> lk2(mChainLock);
    return mResults;
  }

  ComputationChain &operator<<(Processor &processor) {
    addProcessor(processor);
    return *this;
  }

  template <class ParameterType>
  ComputationChain &
  registerParameter(al::ParameterWrapper<ParameterType> &param) {
    mParameters.push_back(&param);
    configuration[param.getName()] = param.get();
    param.registerChangeCallback([&](ParameterType value) {
      configuration[param.getName()] = value;
      process();
    });
    return *this;
  }

  template <class ParameterType>
  ComputationChain &operator<<(al::ParameterWrapper<ParameterType> &newParam) {
    return registerParameter(newParam);
  }
  /**
   * @brief Return list of currently registered processors.
   */
  std::vector<Processor *> processors() { return mProcessors; }

private:
  std::map<std::string, bool> mResults;
  std::vector<Processor *> mProcessors;
  std::vector<ProcessorAsync *> mAsyncProcessesInternal;
  // TODO check, this lock might not be needed as Processor already has a mutex
  // doing this.
  std::mutex mChainLock;
  ChainType mType;
};

} // namespace tinc

#endif // COMPUTATIONCHAIN_HPP
