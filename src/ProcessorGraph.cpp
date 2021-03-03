#include "tinc/ProcessorGraph.hpp"

#include <iostream>

using namespace tinc;

void ProcessorGraph::addProcessor(Processor &proc, bool connectFiles) {
  std::unique_lock<std::mutex> lk(mChainLock);
  switch (mType) {
  case PROCESS_ASYNC:
    // FIXME check if process is already async, so there's no need to do this.
    // FIXME free this on destructor
    mAsyncProcessesInternal.emplace_back(new ProcessorAsyncWrapper(&proc));
    mProcessors.push_back({mAsyncProcessesInternal.back(), false});
    break;
  case PROCESS_SERIAL:
    mProcessors.push_back({&proc, connectFiles});
    break;
  }
}

bool ProcessorGraph::process(bool forceRecompute) {
  mResults.clear();
  if (!enabled) {
    // TODO should callbacks be called if disabled?
    //    callDoneCallbacks(true);
    if (mVerbose) {
      std::cerr << "SKIPPED: " << mId << " - processor disabled" << std::endl;
    }
    return true;
  }

  if (mVerbose) {
    std::cerr << "STARTING Processor Graph: " << mId << std::endl;
  }
  callStartCallbacks();
  std::unique_lock<std::mutex> lk2(mChainLock);
  std::unique_lock<std::mutex> lk(mProcessLock);
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << mId << std::endl;
    return false;
  }
  bool ret = true;
  bool thisRet = true;
  switch (mType) {
  case PROCESS_ASYNC:
    for (auto proc : mProcessors) {
      for (auto configEntry : configuration) {
        proc.first->configuration[configEntry.first] = configEntry.second;
      }
      mResults[proc.first->getId()] = proc.first->process(forceRecompute);
    }
    for (auto proc : mProcessors) {
      thisRet = ((ProcessorAsyncWrapper *)proc.first)->waitUntilDone();
      mResults[proc.first->getId()] = thisRet;
      if (!proc.first->ignoreFail) {
        if (!proc.first->ignoreFail) {
          ret &= thisRet;
        }
      }
    }
    break;
  case PROCESS_SERIAL:
    for (auto proc : mProcessors) {
      proc.first->setRunningDirectory(this->getRunningDirectory());
      for (auto configEntry : configuration) {
        proc.first->configuration[configEntry.first] = configEntry.second;
      }
      thisRet = proc.first->process(forceRecompute);
      mResults[proc.first->getId()] = thisRet;
      if (!proc.first->ignoreFail) {
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

std::map<std::string, bool> ProcessorGraph::getResults() {
  std::unique_lock<std::mutex> lk2(mChainLock);
  return mResults;
}

ProcessorGraph &tinc::ProcessorGraph::operator<<(Processor &processor) {
  addProcessor(processor);
  return *this;
}

std::vector<Processor *> ProcessorGraph::getProcessors() {
  std::vector<Processor *> procs;
  std::unique_lock<std::mutex> lk2(mChainLock);
  for (auto proc : mProcessors) {
    procs.push_back(proc.first);
  }
  return procs;
}
