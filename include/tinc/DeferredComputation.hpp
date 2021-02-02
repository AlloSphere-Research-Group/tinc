#ifndef DEFERREDCOMPUTATION_HPP
#define DEFERREDCOMPUTATION_HPP

/*
 * Copyright 2020 AlloSphere Research Group
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 *        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * authors: Andres Cabrera
*/

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "tinc/BufferManager.hpp"
// -----------------------------------------

namespace tinc {

template <class DataType>
class DeferredComputation : public BufferManager<DataType> {
public:
  DeferredComputation(uint16_t size = 2) : BufferManager<DataType>(size) {}

  ~DeferredComputation() {
    if (mProcessingThread) {
      mProcessingThread->join();
    }
  }

  //  template<typename ...ProcessParams>
  //  bool process(bool(*func)(std::shared_ptr<DataType>, ProcessParams... ),
  //               ProcessParams... params) {
  //    // TODO do a try lock and fail gracefully to avoid accumulating process
  //    calls?

  //    std::cout << "Starting process" << std::endl;
  //    std::unique_lock<std::mutex> lk(mProcessLock);
  //    uint16_t freeBuffer = (mReadBuffer + 1) % mSize;
  //    if (mData[freeBuffer].use_count() == 1 ) { // TODO wait a bit or block
  //    for the release of buffer?
  //      if (func(mData[freeBuffer], params... )) {
  //        mNewData = true;
  //        mReadBuffer = (mReadBuffer + 1) % mSize;
  //        std::cout << "Ending process - ok" << std::endl;
  //        return true;
  //      } else {
  //        std::cerr << "ERROR: Function returned false" << std::endl;
  //      }
  //    } else {
  //      std::cerr << "ERROR: Ignoring process request as target buffer busy"
  //      << std::endl;
  //    }
  //    std::cout << "Ending process - failed" << std::endl;
  //    return false;
  //  }

  template <typename Function, typename... ProcessParams>
  bool process(Function func, ProcessParams... params) {
    // TODO do a try lock and fail gracefully to avoid accumulating process
    // calls?

    std::cout << "Starting process" << std::endl;
    std::unique_lock<std::mutex> lk(mProcessLock);
    uint16_t freeBuffer = (BufferManager<DataType>::mReadBuffer + 1) %
                          BufferManager<DataType>::mSize;
    if (BufferManager<DataType>::mData[freeBuffer].use_count() ==
        1) { // TODO wait a bit or block for the release of buffer?
      if (func(BufferManager<DataType>::mData[freeBuffer], params...)) {
        BufferManager<DataType>::mNewData = true;
        BufferManager<DataType>::mReadBuffer =
            (BufferManager<DataType>::mReadBuffer + 1) %
            BufferManager<DataType>::mSize;
        std::cout << "Ending process - ok" << std::endl;
        return true;
      } else {
        std::cerr << "ERROR: Function returned false" << std::endl;
      }
    } else {
      std::cerr << "ERROR: Ignoring process request as target buffer busy"
                << std::endl;
    }
    std::cout << "Ending process - failed" << std::endl;
    return false;
  }

  template <typename Function, typename... ProcessParams>
  void processAsync(Function &&func, ProcessParams &&... params) {
    std::unique_lock<std::mutex> lk(mThreadLock);
    if (mProcessingThread) {
      mProcessingThread->join();
    }

    std::unique_lock<std::mutex> lck(mAsyncMutex);
    mProcessingThread = std::make_unique<std::thread>([&]() {
      std::unique_lock<std::mutex> lck(mAsyncMutex);
      mProcessing = true;
      mAsyncSignal.notify_all();
      process(std::forward<Function>(func),
              std::forward<ProcessParams>(params)...);
      mProcessing = false;
    });
    mAsyncSignal.wait(lck); // Wait for thread to start
  }

  std::mutex mAsyncMutex;
  std::condition_variable mAsyncSignal;

  //  template<typename ...ProcessParams>
  //  void processAsync(bool(*func)(std::shared_ptr<DataType>, ProcessParams...
  //  ),
  //                    std::function<void(bool)> callback,
  //                    ProcessParams... params) {
  //    std::unique_lock<std::mutex> lk(mThreadLock);
  //    if (mProcessingThread) {
  //      mProcessingThread->join();
  //    }
  //    mProcessingThread = std::make_unique<std::thread>([&, this]() {
  //      bool ok = process(func, params...);
  //      callback(ok);
  //    });
  //  }

  bool processing() { return mProcessing; }

private:
  std::mutex mThreadLock;
  // TODO allow multiple threads
  std::shared_ptr<std::thread> mProcessingThread;
  std::atomic<bool> mProcessing;
  std::mutex mProcessLock;
};

} // namespace tinc

#endif // DEFERREDCOMPUTATION_HPP
