#ifndef PROCESSORASYNCWRAPPER_HPP
#define PROCESSORASYNCWRAPPER_HPP

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

#include "tinc/Processor.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace tinc {

/**
 * @brief Wraps a Processor object to run it asynchronously.
 *
 * This class is needed as there are cases where the processor might be running
 * both synchronously and asynchronously, for example when requesting a
 * synchronous
 * run while there is an asynchronous parameter sweep taking place.
 */
class ProcessorAsyncWrapper : public Processor {
public:
  ProcessorAsyncWrapper(std::string id);

  ProcessorAsyncWrapper(Processor *processor = nullptr);

  ~ProcessorAsyncWrapper();

  // FIXME we need to take over the other process if we turn it async.
  ProcessorAsyncWrapper(const ProcessorAsyncWrapper &other) // copy constructor
      : ProcessorAsyncWrapper(other.mProcessor) {
    mId = other.mId;
  }
  ProcessorAsyncWrapper(
      ProcessorAsyncWrapper &&other) noexcept // move constructor
      : mThread(std::exchange(other.mThread, nullptr)) {
    mId = other.mId;
  }
  ProcessorAsyncWrapper &
  operator=(const ProcessorAsyncWrapper &other) // copy assignment
  {
    return *this = ProcessorAsyncWrapper(other);
    ;
  }
  ProcessorAsyncWrapper &
  operator=(ProcessorAsyncWrapper &&other) noexcept // move assignment
  {
    mId = other.mId;
    std::swap(mThread, other.mThread);
    return *this;
  }

  bool process(bool forceRecompute = false) override;
  bool waitUntilDone();

  Processor *processor() const;
  void setProcessor(Processor *processor);

protected:
  void startThread();

private:
  Processor *mProcessor{nullptr};
  bool mRunning{true};
  bool mRequestForce{false};
  bool mRetValue;
  std::unique_ptr<std::thread> mThread;
  std::mutex mLock;
  std::condition_variable mCondVariable;

  std::mutex mStartLock;
  std::condition_variable mStartCondVariable;
};

} // namespace tinc

#endif // PROCESSORASYNCWRAPPER_HPP
