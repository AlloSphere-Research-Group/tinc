#include "al/system/al_Time.hpp"

#include "tinc/ProcessorAsyncWrapper.hpp"

#include <iostream>
#include <thread>

using namespace tinc;

ProcessorAsyncWrapper::ProcessorAsyncWrapper(std::string id) : Processor(id) {
  startThread();
}

ProcessorAsyncWrapper::ProcessorAsyncWrapper(Processor *processor)
    : Processor(processor->getId()), mProcessor(processor) {
  startThread();
}

ProcessorAsyncWrapper::~ProcessorAsyncWrapper() {
  mRunning = false;
  mThread->join();
}

bool ProcessorAsyncWrapper::process(bool forceRecompute) {
  {
    std::unique_lock<std::mutex> lk(mLock);
    //    std::cout << "start " << mProcessor->id << std::endl;
    mRequestForce = forceRecompute;
    //    std::cout << "notifying thread start " << mProcessor->id << std::endl;
    mCondVariable.notify_one();
  }
  {
    std::unique_lock<std::mutex> lk2(mStartLock);
    mStartCondVariable.wait(lk2);
  }
  return true;
}

bool ProcessorAsyncWrapper::waitUntilDone() {
  // As soon as we can acquire the lock, the thread is waiting
  mLock.lock();
  //  std::cout << "done " << mProcessor->id << std::endl;
  mLock.unlock();
  return mRetValue;
}

void ProcessorAsyncWrapper::startThread() {

  mThread = std::make_unique<std::thread>([this]() {
    std::unique_lock<std::mutex> lk(mLock);
    while (mRunning) {
      mCondVariable.wait(lk);
      {
        std::unique_lock<std::mutex> lk(mStartLock);
        mStartCondVariable.notify_one();
      }
      //      std::cout << "starting thread process " << mProcessor->id <<
      //      std::endl;
      if (mProcessor) {
        mRetValue = mProcessor->process(mRequestForce);
      } else {
        mRetValue = this->process(mRequestForce);
      }
      callDoneCallbacks(mRetValue);
      //      std::cout << "completed thread process " << mProcessor->id <<
      //      std::endl;
    }
  });
}

Processor *ProcessorAsyncWrapper::processor() const { return mProcessor; }

void ProcessorAsyncWrapper::setProcessor(Processor *processor) {
  mProcessor = processor;
}
