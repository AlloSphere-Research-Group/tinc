#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

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

#include "tinc/IdObject.hpp"
#include "tinc/ParameterSpaceDimension.hpp"
#include "tinc/VariantValue.hpp"

#include "al/scene/al_PolySynth.hpp"

#include <string>
#include <vector>

namespace tinc {

constexpr auto PROCESSOR_META_FORMAT_VERSION = 0;

/**
 * @brief The Processor class presents an abstraction to filesystem based
 * computation
 *
 * An instance of Processor can only run a single instance of its process()
 * function.
 */
class Processor : public IdObject {
  friend class ProcessorGraph;

public:
  Processor(std::string id_ = "") { setId(id_); }
  Processor(Processor &p)
      : mInputDirectory(p.mInputDirectory),
        mOutputDirectory(p.mOutputDirectory),
        mRunningDirectory(p.mRunningDirectory),
        mOutputFileNames(p.mOutputFileNames),
        mInputFileNames(p.mInputFileNames) {}

  virtual ~Processor() {}

  /**
   * @brief override this function to determine how subclasses should process
   *
   * You must call prepareFunction(), callDoneCallbacks() , callStartCallbacks()
   * and test for 'enabled' within the process() function of all child classes.
   */
  virtual bool process(bool forceRecompute = false) = 0;

  /**
   * @brief returns true is the process() function is currently running
   */
  bool isRunning();

  /**
   * @brief Convenience function to set input and output directory
   */
  void setDataDirectory(std::string directory);

  /**
   * @brief Set the directory for output files
   */
  void setOutputDirectory(std::string outputDirectory);

  /**
   * @brief Get the directory for output files
   */
  std::string getOutputDirectory() { return mOutputDirectory; }

  /**
   * @brief Set the directory for input files
   */
  void setInputDirectory(std::string inputDirectory);

  /**
   * @brief Get the directory for input files
   */
  std::string getInputDirectory() { return mInputDirectory; }

  /**
   * @brief Set the names of output files
   * @param outputFiles list of output file names.
   */
  void setOutputFileNames(std::vector<std::string> outputFiles);

  /**
   * @brief Query the current output filenames
   *
   * This function contains the filenames set in C++ using setOutputFileNames or
   * the filenames set by the Processor after running, see for example
   * ProcessorScript
   */
  std::vector<std::string> getOutputFileNames();

  /**
   * @brief Set the names of input files
   * @param outputFiles list of output file names.
   */
  void setInputFileNames(std::vector<std::string> inputFiles);

  /**
   * @brief Query the current input filenames
   *
   * This function contains the filenames set in C++ using setInputFileNames or
   * the filenames set by the Processor after running, see for example
   * ProcessorScript
   */
  std::vector<std::string> getInputFileNames();

  /**
   * @brief Set the current directory for process to run in.
   */
  void setRunningDirectory(std::string directory);

  /**
   * @brief Get the directory for input files
   */
  std::string getRunningDirectory() { return mRunningDirectory; }

  /**
   * @brief Register a function to be called at the start of process()
   *
   * These callbacks should only be used for data related actions, as they
   * will be skipped when reusing cache in ParameterSpace.
   */
  void registerStartCallback(std::function<void(void)> func);

  /**
   * @brief Register a function to be called at the end of process()
   *
   * This function is passed true if the process() function has been sucessful.
   *
   * These callbacks should only be used for data related actions, as they
   * will be skipped when reusing cache in ParameterSpace.
   */
  void registerDoneCallback(std::function<void(bool)> func);

  void setVerbose(bool verbose = true) { mVerbose = verbose; }

  /**
   * If set to true, ProcessorChains will continue even if this processor
   * fails. Has no effect if running chain asychronously
   */
  bool ignoreFail{false};

  /**
   * If set to false, process() has no effect and will return true;
   */
  bool enabled{true};

  /**
   * @brief Set a function to be called before computing to prepare data
   *
   * When writing the prepare function you should access values and ids through
   * Processor::configuration. If you access values directly from dimensions,
   * you will likely break ParameterSpace::sweep used with this Processor as
   * sweep() does not change the internal values of the parameter space and its
   * dimensions.
   */
  std::function<bool(void)> prepareFunction;

  /**
   * @brief Register a dimension so that the Processor is executed on changes
   */
  Processor &registerDimension(ParameterSpaceDimension &dim);

  /**
   * @brief Convenient syntax for registerDimension()
   */
  Processor &operator<<(ParameterSpaceDimension &dim) {
    return registerDimension(dim);
  }

  /**
   * @brief Enable simple file based caching
   * @param use
   *
   * This caching is a simple but brittle caching mechanism. Whenever possible
   * use the caching provided by ParameterSpace, as it is more robust.
   *
   * For this caching to work, you will need to ensure that every independent
   * run produces unique and reproducible filenames. i.e. you will need to set
   * the output file names on every run to include in their name all parameter
   * values and dependencies, or have some other way of connecting the
   * parameters/ parameter space to the file.
   *
   * This will cache the output as written in the output files. You can set and
   * query the files using getOutputFiles() setOutputFiles().
   */
  void useCache(bool use = true) { mUseCache = use; }

  /**
   * @brief Register a Parameter so that the Processor is executed on changes
   */
  template <class ParameterType>
  Processor &registerParameter(al::ParameterWrapper<ParameterType> &param) {
    mParameters.push_back(&param);
    configuration[param.getName()] = param.get();
    param.registerChangeCallback([&](ParameterType value) {
      configuration[param.getName()] = value;
      process();
    });
    return *this;
  }

  /**
   * @brief Convenient syntax for registerParameter()
   */
  template <class ParameterType>
  Processor &operator<<(al::ParameterWrapper<ParameterType> &newParam) {
    return registerParameter(newParam);
  }

  typedef std::map<std::string, VariantValue> Configuration;

  /**
   * @brief Current internal configuration key value pairs
   *
   * Reflects the most recently used configuration (whether successful or
   * failed) or the configuration for the currently running process.
   */
  Configuration configuration;

protected:
  std::string mInputDirectory;
  std::string mOutputDirectory;
  std::string mRunningDirectory;
  std::vector<std::string> mOutputFileNames;
  std::vector<std::string> mInputFileNames;
  bool mVerbose;

  bool mUseCache{false};

  std::vector<al::ParameterMeta *> mParameters;

  void callStartCallbacks();
  void callDoneCallbacks(bool result);

  // Metadata
  virtual bool writeMeta();
  virtual bool needsRecompute();
  std::string metaFilename();

  std::mutex mProcessLock;

private:
  std::vector<std::function<void()>> mStartCallbacks;
  std::vector<std::function<void(bool)>> mDoneCallbacks;
};

} // namespace tinc

#endif // PROCESSOR_HPP
