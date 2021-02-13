#include "tinc/ProcessorCpp.hpp"

#include "al/io/al_File.hpp"

using namespace tinc;

ProcessorCpp::ProcessorCpp(std::string id) : Processor(id) {}

bool ProcessorCpp::process(bool forceRecompute) {
  callStartCallbacks();
  al::PushDirectory dir(mRunningDirectory, mVerbose);
  if (!enabled) {
    return true;
  }
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << mId << std::endl;
    return false;
  }
  bool ret = true;
  if (forceRecompute) {
    ret = processingFunction();
  }
  callDoneCallbacks(ret);
  return ret;
}
