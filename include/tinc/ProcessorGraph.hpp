#ifndef COMPUTATIONCHAIN_HPP
#define COMPUTATIONCHAIN_HPP

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

#include "tinc/ProcessorAsyncWrapper.hpp"
#include "tinc/ProcessorScript.hpp"

#include <mutex>
#include <thread>

namespace tinc {

class ProcessorGraph : public Processor {
public:
  typedef enum { PROCESS_SERIAL, PROCESS_ASYNC } ChainType;
  ProcessorGraph(ChainType type = PROCESS_SERIAL, std::string id = "")
      : Processor(id), mType(type) {}
  ProcessorGraph(std::string id) : Processor(id), mType(PROCESS_SERIAL) {}

  void addProcessor(Processor &chain);

  bool process(bool forceRecompute = false) override;

  /**
   * @brief get map of results of computation by processor name
   *
   * This function will block if computation is currently executing and will
   * return after computation is done.
   */
  std::map<std::string, bool> getResults();

  ProcessorGraph &operator<<(Processor &processor);

  template <class ParameterType>
  ProcessorGraph &
  registerParameter(al::ParameterWrapper<ParameterType> &param);

  template <class ParameterType>
  ProcessorGraph &operator<<(al::ParameterWrapper<ParameterType> &newParam) {
    return registerParameter(newParam);
  }
  /**
   * @brief Return list of currently registered processors.
   */
  std::vector<Processor *> processors() { return mProcessors; }

private:
  std::map<std::string, bool> mResults;
  std::vector<Processor *> mProcessors;
  std::vector<ProcessorAsyncWrapper *> mAsyncProcessesInternal;
  // TODO check, this lock might not be needed as Processor already has a mutex
  // doing this.
  std::mutex mChainLock;
  ChainType mType;
};

// Implementation

template <class ParameterType>
ProcessorGraph &ProcessorGraph::registerParameter(
    al::ParameterWrapper<ParameterType> &param) {
  mParameters.push_back(&param);
  configuration[param.getName()] = param.get();
  param.registerChangeCallback([&](ParameterType value) {
    configuration[param.getName()] = value;
    process();
  });
  return *this;
}

} // namespace tinc

#endif // COMPUTATIONCHAIN_HPP
