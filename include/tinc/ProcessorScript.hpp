#ifndef DATASCRIPT_HPP
#define DATASCRIPT_HPP

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

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/Processor.hpp"
#include "tinc/ParameterSpace.hpp"

#include "nlohmann/json.hpp"

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace tinc {

/**
 * @brief The ProcessorScript class
 *
 *
 * @code
ProcessorScript proc{"proc"};

proc.setDataDirectory("/data/dir");
proc.setRunningDirectory("/running/dir");
proc.setInputFileNames({"input_file"});
proc.setOutputFileNames({"output_file"});

proc.setCommand("python");
proc.setScriptName("script.py");

if (proc.process()) {
    // Data is ready
}
@endcode
 *
 * The script will be passed options through a json file, and can
 * also recieve feedback from the script through the same file. The
 * filename is passed to the script as its first argument.
 *
 */
class ProcessorScript : public Processor {
public:
  ProcessorScript(std::string id = "") : Processor(id) {}

  virtual ~ProcessorScript() {}

  // Copy constructor
  ProcessorScript(ProcessorScript &p)
      : Processor(p), mScriptCommand(p.mScriptCommand),
        mScriptName(p.mScriptName) {}

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
  std::string getScriptFile(bool fullPath = false);

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

  /**
   * @brief if set to false disables creation and argument passing of json
   * config
   * @param enable
   */
  void enableJsonConfig(bool enable = true);

  /**
   * @brief
   * @param commandLineTemplate
   *
   * If a template is set, it will be populated by parameters and file names
   * and passed to the command as the command line flags. If the template is
   * used, the json config file is still created, unless enableJsonConfig(false)
   * is called
   */
  void setCommandLineFlagTemplate(std::string commandLineTemplate) {
    tinc::ParameterSpace ps{"PS"};
    std::string mCommandLine;
    std::string line_input, line_output, line_output_dir, line_input_dir;
    std::string chunk;
    std::size_t pos_input, pos_output, pos_output_dir, pos_input_dir;
    std::vector <std::string> input, output, input_dir, output_dir;

    mCommandLine = ps.resolveFilename(commandLineTemplate);
    pos_input = mCommandLine.find("INPUT:");
    pos_output = mCommandLine.find("OUTPUT:");
    pos_output_dir = mCommandLine.find("OUTPUT_DIR:");
    pos_input_dir = mCommandLine.find("INPUT_DIR:");

    if(pos_input!=std::string::npos){
      line_input = mCommandLine.substr(0,pos_input);
      std::stringstream check(line_input);
      while(std::getline(check, chunk, ' ')){
        input.push_back(chunk);
      }
    }else if(pos_output!=std::string::npos){
      line_output = mCommandLine.substr(0,pos_output);
      std::stringstream check(line_output);
      while(std::getline(check, chunk, ' ')){
        output.push_back(chunk);
      }
    }else if(pos_output_dir!=std::string::npos){
      line_output_dir = mCommandLine.substr(0,pos_output_dir);
      std::stringstream check(line_output_dir);
      while(std::getline(check, chunk, ' ')){
        output_dir.push_back(chunk);
      }
    }else if(pos_input_dir!=std::string::npos){
      line_input_dir = mCommandLine.substr(0,pos_input_dir);
      std::stringstream check(line_input_dir);
      while(std::getline(check, chunk, ' ')){
        input_dir.push_back(chunk);
      }
    }
    // TODO ML use ParameterSpace::resolveFilename() to resolve template
    // parameters Then use addtional markers like &&INPUT:0&& to get input file
    // 0 &&INPUT: && to get all input names separated by space or &&INPUT:,&& to
    // separate by commas. Apart from INPUT, support OUTPUT, INPUT_DIR,
    // OUTPUT_DIR. Done. need test.
  }

protected:
  std::string writeJsonConfig();

  // Read configuration from disk. The python script can write configuration to
  // override the configuration provided
  bool readJsonConfig(std::string filename);

  void parametersToConfig(nlohmann::json &j);

private:
  std::string mScriptCommand{"/usr/bin/python3"};
  std::string mScriptName;
  bool mEnableJsonConfig{true};

  std::mutex mProcessingLock;
  int mMaxAsyncProcesses{4};
  std::atomic<int> mNumAsyncProcesses{0};
  std::vector<std::thread> mAsyncThreads;
  std::thread mAsyncDoneThread;
  std::condition_variable mAsyncDoneTrigger;
  std::mutex mAsyncDoneTriggerLock;

  std::string makeCommandLine();

  bool runCommand(const std::string &command);

  bool writeMeta() override;

  bool needsRecompute() override;
};

} // namespace tinc

#endif // DATASCRIPT_HPP
