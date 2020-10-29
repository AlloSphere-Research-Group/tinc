#ifndef DATASCRIPT_HPP
#define DATASCRIPT_HPP

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/Processor.hpp"

#include "nlohmann/json.hpp"

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace tinc {

/**
 * @brief The DataScript class
 *
 *
 * @code
 *

 *
 * @endcode
 *
 *
 * @codeline
 * python myscript.py output_dir option 3.14159 0.5
 *
 * In the /home/sweet/home directory.
 *
 */
class ScriptProcessor : public Processor {
public:
  // TODO change constructor to match Processor constructor
  ScriptProcessor(std::string id = "") : Processor(id) {}

  virtual ~ScriptProcessor() {}

  // Copy constructor
  ScriptProcessor(ScriptProcessor &p)
      : Processor(p), mScriptCommand(p.mScriptCommand), mScriptName(p.mScriptName) {}

  /**
   * @brief Set the script's main command (e.g. python)
   */
  void setCommand(std::string command) { mScriptCommand = command; }

  /**
   * @brief Get the script's main command (e.g. python)
   */
  std::string getCommand() { return mScriptCommand; }

  /**
   * @brief Set name of script to be run
   * @param scriptName
   *
   * This name will be used unless a flag is set with type FLAG_SCRIPT
   */
  void setScriptName(std::string scriptName) {
    std::replace(mScriptName.begin(), mScriptName.end(), '\\', '/');
    mScriptName = scriptName;
  }

  /**
   * @brief Get name of script to be run
   */
  std::string getScriptName() { return mScriptName; }

  /**
   * @brief Query current script file name
   */
  std::string scriptFile(bool fullPath = false);

  /**
   * @brief Query input file name
   */
  std::string inputFile(bool fullPath = true, int index = 0);

  /**
   * @brief Query output file name
   */
  std::string outputFile(bool fullPath = true, int index = 0);

  /**
   * @brief process
   */
  bool process(bool forceRecompute = false) override;

  /**
   * @brief Cleans a name up so it can be written to disk
   * @param output_name the source name
   * @return the cleaned up name
   *
   * This function will remove any characters not allowed by the operating
   * system like :,/,\ etc. And will remove any characters like '.' that
   * can confuse the parsing of the name on read.
   */
  static std::string sanitizeName(std::string output_name);

  //  // TODO remove these async calls
  //  bool processAsync(bool noWait = false,
  //                    std::function<void(bool)> doneCallback = nullptr);

  //  bool processAsync(std::map<std::string, std::string> options,
  //                    bool noWait = false,
  //                    std::function<void(bool)> doneCallback = nullptr);

  //  bool runningAsync();

  //  bool waitForAsyncDone();

  //  void maxAsyncProcesses(int num) { mMaxAsyncProcesses = num; }

protected:
  std::string writeJsonConfig();

  void parametersToConfig(nlohmann::json &j);

private:
  std::string mScriptCommand{"/usr/bin/python3"};
  std::string mScriptName;

  std::mutex mProcessingLock;
  int mMaxAsyncProcesses{4};
  std::atomic<int> mNumAsyncProcesses{0};
  std::vector<std::thread> mAsyncThreads;
  std::thread mAsyncDoneThread;
  std::condition_variable mAsyncDoneTrigger;
  std::mutex mAsyncDoneTriggerLock;

  std::string makeCommandLine();

  bool runCommand(const std::string &command);

  bool writeMeta();

  al_sec modified(const char *path) const;

  bool needsRecompute();

  std::string metaFilename();
};

} // namespace tinc

//#ifdef AL_WINDOWS
//#undef popen
//#undef pclose
//#endif

#endif // DATASCRIPT_HPP
