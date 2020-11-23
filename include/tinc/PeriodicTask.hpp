#ifndef PERIODICTASK_HPP
#define PERIODICTASK_HPP

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

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

using namespace std::chrono_literals;

namespace tinc {

class PeriodicTask {
public:
  ~PeriodicTask() { stop(); }

  bool start(std::function<bool()> func) {
    if (mFuncThread) {
      return false;
    }
    mRunning = true;
    mFuncThread = std::make_shared<std::thread>(threadFunction, func, this);
    return mFuncThread != nullptr;
  }

  bool running() { return mRunning || mFuncThread != nullptr; }

  void stop() {
    if (mFuncThread) {
      mRunning = false;
      mFuncThread->join();
      mFuncThread = nullptr;
    }
  }

  std::chrono::nanoseconds waitTime() const;
  void setWaitTime(std::chrono::nanoseconds waitTime);

  //  std::chrono::nanoseconds granularity() const
  //  {
  //    return mGranularity;
  //  }
  //  void setGranularity(std::chrono::nanoseconds granularity)
  //  {
  //    mGranularity = granularity;
  //  }

private:
  static void threadFunction(std::function<bool()> func,
                             PeriodicTask *thisPtr) {
    auto waitTime = thisPtr->waitTime();
    while (thisPtr->mRunning) {
      if (!func()) {
        thisPtr->mRunning = false;
        break;
      }
      std::this_thread::sleep_for(waitTime);
    }
  }

  std::chrono::nanoseconds mWaitTime{10000ns};
  // Fixme implement granularity for snappier shutdown of waiting
  std::chrono::nanoseconds mGranularity{10000ns};
  std::atomic<bool> mRunning;
  std::shared_ptr<std::thread> mFuncThread;
};

std::chrono::nanoseconds PeriodicTask::waitTime() const { return mWaitTime; }

void PeriodicTask::setWaitTime(std::chrono::nanoseconds waitTime) {
  mWaitTime = waitTime;
}

} // namespace tinc

#endif // PERIODICTASK_HPP
