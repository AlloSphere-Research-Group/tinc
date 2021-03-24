#include "tinc/ProcessorCpp.hpp"

#include "al/io/al_File.hpp"

using namespace tinc;

ProcessorCpp::ProcessorCpp(std::string id) : Processor(id) {}

bool ProcessorCpp::process(bool forceRecompute) {
  callStartCallbacks();
  if (!enabled) {
    return true;
  }
  if (prepareFunction) {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    if (!prepareFunction()) {
      std::cerr << "ERROR preparing processor: " << mId << std::endl;
      return false;
    }
  }
  bool ret = true;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    if (needsRecompute() || forceRecompute) {
      ret = processingFunction();
    }
  }
  callDoneCallbacks(ret);
  return ret;
}
