#ifndef COMPUTATIONCHAIN_HPP
#define COMPUTATIONCHAIN_HPP

/*
 * Copyright 2020, 2021 AlloSphere Research Group
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
/**
 * @brief A class to trigger a group of Processor classes in series or in
 * parallel
 *
 * Complex graphs can be created by connecting and nesting ProcessorGraph
 * objects
 */
class ProcessorGraph : public Processor {
public:
  typedef enum { PROCESS_SERIAL, PROCESS_ASYNC } ChainType;
  ProcessorGraph(ChainType type = PROCESS_SERIAL, std::string id = "")
      : Processor(id), mType(type) {}
  ProcessorGraph(std::string id) : Processor(id), mType(PROCESS_SERIAL) {}

  /**
   * @brief Add a processor to the computation chain
   * @param proc
   * @param connectFiles true if output files should be connected to input files
   *
   * connectFiles only has effect if ChainType is PROCESS_SERIAL, it is ingored
   * for PROCESS_ASYNC
   */
  void addProcessor(Processor &proc, bool connectFiles = true);

  /**
   * @brief Syntactic sugar for addProcessor()
   *
   * Call addProcessor() with connectFiles set to true
   */
  ProcessorGraph &operator<<(Processor &processor);

  bool process(bool forceRecompute = false) override;

  /**
   * @brief get map of results of computation by processor name
   *
   * This function will block if computation is currently executing and will
   * return after computation is done.
   */
  std::map<std::string, bool> getResults();

  //  /**
  //   * Register a parameter to trigger computation. The parameter value is
  //   also
  //   * automatically added to the configuration passed to children Processors.
  //   * These values will override any values of the same name that have been
  //   set.
  //   * (for example from a parameter sweep or manually by the user)
  //   */
  //  template <class ParameterType>
  //  ProcessorGraph &registerParameter(al::ParameterWrapper<ParameterType>
  //  &param);

  //  /**
  //   * @brief Syntactic sugar for registerParameter()
  //   */
  //  template <class ParameterType>
  //  ProcessorGraph &operator<<(al::ParameterWrapper<ParameterType> &newParam)
  //  {
  //    return registerParameter(newParam);
  //  }
  /**
   * @brief Return list of currently registered processors.
   */
  std::vector<Processor *> getProcessors();

private:
  std::map<std::string, bool> mResults;
  std::mutex mChainLock; // Exclusive access to processor list mProcessors
  std::vector<std::pair<Processor *, bool>> mProcessors;
  std::vector<ProcessorAsyncWrapper *> mAsyncProcessesInternal;
  ChainType mType;
};

// Implementation

// template <class ParameterType>
// ProcessorGraph &
// ProcessorGraph::registerParameter(al::ParameterWrapper<ParameterType> &param)
// {
//  mParameters.push_back(&param);
//  configuration[param.getName()] = param.get();
//  param.registerChangeCallback([&](ParameterType value) {
//    configuration[param.getName()] = value;
//    process();
//  });
//  return *this;
//}

} // namespace tinc

#endif // COMPUTATIONCHAIN_HPP
