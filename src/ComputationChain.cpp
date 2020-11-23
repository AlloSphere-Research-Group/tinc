#include "tinc/ComputationChain.hpp"

#include <iostream>

using namespace tinc;

void ComputationChain::addProcessor(Processor &chain) {
  std::unique_lock<std::mutex> lk(mChainLock);
  switch (mType) {
  case PROCESS_ASYNC:
    // FIXME check if process is already async, so there's no need to do this.
    // FIXME free this on destructor
    mAsyncProcessesInternal.emplace_back(new ProcessorAsyncWrapper(&chain));
    mProcessors.push_back(mAsyncProcessesInternal.back());
    break;
  case PROCESS_SERIAL:
    mProcessors.push_back(&chain);
    break;
  }
}

bool ComputationChain::process(bool forceRecompute) {
  mResults.clear();
  if (!enabled) {
    // TODO should callbacks be called if disabled?
    //    callDoneCallbacks(true);
    return true;
  }

  std::unique_lock<std::mutex> lk(mProcessLock);
  std::unique_lock<std::mutex> lk2(mChainLock);
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << mId << std::endl;
    return false;
  }
  bool ret = true;
  bool thisRet = true;
  switch (mType) {
  case PROCESS_ASYNC:
    for (auto chain : mProcessors) {
      for (auto configEntry : configuration) {
        chain->configuration[configEntry.first] = configEntry.second;
      }
      mResults[chain->getId()] = chain->process(forceRecompute);
    }
    for (auto chain : mProcessors) {
      thisRet = ((ProcessorAsyncWrapper *)chain)->waitUntilDone();
      mResults[chain->getId()] = thisRet;
      if (!chain->ignoreFail) {
        if (!chain->ignoreFail) {
          ret &= thisRet;
        }
      }
    }
    break;
  case PROCESS_SERIAL:
    for (auto chain : mProcessors) {
      for (auto configEntry : configuration) {
        chain->configuration[configEntry.first] = configEntry.second;
      }
      thisRet = chain->process(forceRecompute);
      mResults[chain->getId()] = thisRet;
      if (!chain->ignoreFail) {
        ret &= thisRet;
        if (!thisRet) {
          break;
        }
      }
    }
    break;
  }
  callDoneCallbacks(ret);
  return ret;
}
