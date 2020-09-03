#include "tinc/CppProcessor.hpp"

using namespace tinc;

CppProcessor::CppProcessor(std::string id) : Processor(id) {}

bool CppProcessor::process(bool forceRecompute) {
  PushDirectory dir(mRunningDirectory, mVerbose);
  if (!enabled) {
    return true;
  }
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << mId << std::endl;
    return false;
  }
  bool ret = processingFunction();
  callDoneCallbacks(ret);
  return ret;
}
