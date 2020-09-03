#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "tinc/IdObject.hpp"

#include "al/scene/al_PolySynth.hpp"

#include <string>
#include <vector>

namespace tinc {

// TODO move PushDirectory to allolib? Or its own file?
class PushDirectory {
public:
  PushDirectory(std::string directory, bool verbose = false);

  ~PushDirectory();

private:
  char previousDirectory[4096];
  bool mVerbose;

  static std::mutex mDirectoryLock; // Protects all instances of PushDirectory
};

enum FlagType {
  FLAG_INT64 = 0,
  FLAG_DOUBLE, // The script to be run
  FLAG_STRING
};

struct VariantValue {

  VariantValue() {}
  VariantValue(std::string value) {
    type = FLAG_STRING;
    valueStr = value;
  }
  VariantValue(const char *value) {
    type = FLAG_STRING;
    valueStr = value;
  }

  VariantValue(int64_t value) {
    type = FLAG_INT64;
    valueInt = value;
  }

  VariantValue(double value) {
    type = FLAG_DOUBLE;
    valueDouble = value;
  }

  //  ~VariantValue()
  //  {
  //      delete[] cstring;  // deallocate
  //  }

  //  VariantValue(const VariantValue& other) // copy constructor
  //      : VariantValue(other.cstring)
  //  {}

  //  VariantValue(VariantValue&& other) noexcept // move constructor
  //      : cstring(std::exchange(other.cstring, nullptr))
  //  {}

  //  VariantValue& operator=(const VariantValue& other) // copy assignment
  //  {
  //      return *this = VariantValue(other);
  //  }

  //  VariantValue& operator=(VariantValue&& other) noexcept // move assignment
  //  {
  //      std::swap(cstring, other.cstring);
  //      return *this;
  //  }

  std::string commandFlag; // A prefix to the flag (e.g. -o)

  FlagType type;
  std::string valueStr;
  int64_t valueInt;
  double valueDouble;
};

/**
 * @brief The Processor class presents an abstraction to filesystem based
 * computation
 *
 * An instance of Processor can only run a single instance of its process()
 * function.
 */
class Processor : public IdObject {
public:
  typedef std::map<std::string, VariantValue> Configuration;

  Processor(std::string id_ = "") { setId(id_); }
  virtual ~Processor() {}

  /**
   * @brief override this function to determine how subclasses should process
   *
   * You must call prepareFunction(), callDoneCallbacks() and test for 'enabled'
   * within the process() function of all child classes.
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
   */
  std::vector<std::string> getOutputFileNames();

  /**
   * @brief Set the names of input files
   * @param outputFiles list of output file names.
   */
  void setInputFileNames(std::vector<std::string> inputFiles);

  // TODO how should the files synchronize to processing. Should these functions
  // return only available files, or should they reflect the current settings?
  // Perhaps we need two different functions
  /**
   * @brief Query the current input filenames
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

  void registerDoneCallback(std::function<void(bool)> func) {
    mDoneCallbacks.push_back(func);
  }

  void verbose(bool verbose = true) { mVerbose = verbose; }

  std::string id;
  bool ignoreFail{false}; ///< If set to true, processor chains will continue
                          ///< even if this processor fails. Has no effect if
                          ///< running asychronously
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

  template <class ParameterType>
  Processor &operator<<(al::ParameterWrapper<ParameterType> &newParam) {
    return registerParameter(newParam);
  }

  /**
   * @brief Current internal configuration key value pairs
   *
   * Reflects the most recently used configuration (whether successful or
   * failed) or the configuration for the currently running process.
   */
  Configuration configuration;

protected:
  std::string mRunningDirectory;
  std::string mOutputDirectory;
  std::string mInputDirectory;
  std::vector<std::string> mOutputFileNames;
  std::vector<std::string> mInputFileNames;
  bool mVerbose;

  std::vector<al::ParameterMeta *> mParameters;

  void callDoneCallbacks(bool result) {
    for (auto cb : mDoneCallbacks) {
      cb(result);
    }
  }
  std::mutex mProcessLock;

private:
  std::vector<std::function<void(bool)>> mDoneCallbacks;
};

} // namespace tinc

#endif // PROCESSOR_HPP
