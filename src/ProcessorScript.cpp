
#include "tinc/ProcessorScript.hpp"

#include "nlohmann/json.hpp"

#include <array>
#include <atomic>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip> // setprecision
#include <iostream>
#include <mutex>
#include <thread>
#include <utility> // For pair

#if defined(AL_OSX) || defined(AL_LINUX) || defined(AL_EMSCRIPTEN)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(AL_WINDOWS)
#include <Windows.h>
#include <direct.h> // for _chdir() and _getcwd()
#define chdir _chdir
#define getcwd _getcwd

#define popen _popen
#define pclose _pclose
#include <fileapi.h>
#endif

using namespace tinc;

constexpr auto DATASCRIPT_META_FORMAT_VERSION = 0;

std::string ProcessorScript::scriptFile(bool fullPath) { return mScriptName; }

std::string ProcessorScript::inputFile(bool fullPath, int index) {
  std::string inputName;
  if (mInputFileNames.size() > index) {
    inputName = mInputFileNames[index];
  }

  if (fullPath) {
    return getRunningDirectory() + inputName;
  } else {
    return inputName;
  }
}

std::string ProcessorScript::outputFile(bool fullPath, int index) {
  std::string outputName;
  if (mOutputFileNames.size() > index) {
    outputName = mOutputFileNames[index];
  }
  if (fullPath) {
    return getOutputDirectory() + outputName;
  } else {
    return outputName;
  }
}

bool ProcessorScript::process(bool forceRecompute) {
  if (!enabled) {
    return true;
  }
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << getId() << std::endl;
    return false;
  }
  if (mScriptName == "" || mScriptCommand == "") {
    std::cout << "ERROR: process() for '" << getId()
              << "' missing script name or script command." << std::endl;
    return false;
  }
  std::unique_lock<std::mutex> lk(mProcessingLock);

  auto jsonFilename = writeJsonConfig();
  if (jsonFilename.size() == 0) {
    return false;
  }
  bool ok = true;
  if (needsRecompute() || forceRecompute) {
    std::string command =
        mScriptCommand + " \"" + mScriptName + "\" \"" + jsonFilename + "\"";
    ok = runCommand(command);
    if (ok) {
      writeMeta();
    }
  } else {
    if (mVerbose) {
      std::cout << "No need to update cache according to " << metaFilename()
                << std::endl;
    }
  }
  callDoneCallbacks(ok);
  return ok;
}

std::string ProcessorScript::sanitizeName(std::string output_name) {
  std::replace(output_name.begin(), output_name.end(), '/', '_');
  std::replace(output_name.begin(), output_name.end(), '.', '_');
  std::replace(output_name.begin(), output_name.end(), ':', '_');
  std::replace(output_name.begin(), output_name.end(), '\\', '_');
  return output_name;
}

// bool ProcessorScript::processAsync(bool noWait,
//                                   std::function<void(bool)> doneCallback) {
//  std::lock_guard<std::mutex> lk(mProcessingLock);

//  if (needsRecompute()) {
//    std::string command = makeCommandLine();
//    while (mNumAsyncProcesses.fetch_add(1) > mMaxAsyncProcesses) {
//      mNumAsyncProcesses--;
//      if (noWait) {
//        return false; // Async process not started
//      }
//      std::unique_lock<std::mutex> lk2(mAsyncDoneTriggerLock);
//      std::cout << "Async waiting 2 " << mNumAsyncProcesses << std::endl;
//      mAsyncDoneTrigger.wait(lk2);
//      std::cout << "Async done waiting 2 " << mNumAsyncProcesses << std::endl;
//    }
//    //            std::cout << "Async " << mNumAsyncProcesses << std::endl;
//    mAsyncThreads.emplace_back(std::thread([this, command, doneCallback]() {
//      bool ok = runCommand(command);

//      if (doneCallback) {
//        doneCallback(ok);
//      }
//      mNumAsyncProcesses--;
//      mAsyncDoneTrigger.notify_all();
//      //                std::cout << "Async runner done" << mNumAsyncProcesses
//      //                << std::endl;
//    }));

//    PushDirectory p(mRunningDirectory, mVerbose);
//    writeMeta();
//  } else {
//    if (doneCallback) {
//      doneCallback(true);
//    }
//  }
//  return true;
//}

// bool ProcessorScript::processAsync(std::map<std::string, std::string>
// options,
//                                   bool noWait,
//                                   std::function<void(bool)> doneCallback) {
//  std::unique_lock<std::mutex> lk(mProcessingLock);

//  if (needsRecompute()) {
//    lk.unlock();
//    while (mNumAsyncProcesses.fetch_add(1) > mMaxAsyncProcesses) {
//      mNumAsyncProcesses--;
//      if (noWait) {
//        return false; // Async process not started
//      }
//      std::unique_lock<std::mutex> lk2(mAsyncDoneTriggerLock);
//      std::cout << "Async waiting " << mNumAsyncProcesses << std::endl;
//      mAsyncDoneTrigger.wait(lk2);
//      std::cout << "Async done waiting " << mNumAsyncProcesses << std::endl;
//    }
//    //            std::cout << "Async " << mNumAsyncProcesses << std::endl;

//    if (mVerbose) {
//      std::cout << "Starting asyc thread" << std::endl;
//    }
//    mAsyncThreads.emplace_back(std::thread([this, doneCallback]() {
//      bool ok = process(true);

//      PushDirectory p(mRunningDirectory, mVerbose);
//      writeMeta();

//      if (doneCallback) {
//        doneCallback(ok);
//      }
//      mNumAsyncProcesses--;
//      mAsyncDoneTrigger.notify_all();
//      //                std::cout << "Async runner done" <<
//      //                mNumAsyncProcesses << std::endl;
//    }));
//  } else {
//    doneCallback(true);
//  }
//  return true;
//}

// bool ProcessorScript::runningAsync() {
//  if (mNumAsyncProcesses > 0) {
//    return true;
//  } else {
//    return false;
//  }
//}

// bool ProcessorScript::waitForAsyncDone() {
//  bool ok = true;
//  for (auto &t : mAsyncThreads) {
//    t.join();
//  }
//  return ok;
//}

std::string ProcessorScript::writeJsonConfig() {
  using json = nlohmann::json;
  json j;

  j["__tinc_metadata_version"] = DATASCRIPT_META_FORMAT_VERSION;
  j["__output_dir"] = getOutputDirectory();
  j["__output_name"] = outputFile(false);
  j["__input_dir"] = getInputDirectory();
  j["__input_name"] = inputFile(false);

  parametersToConfig(j);

  for (auto c : configuration) {
    if (c.second.type == VARIANT_STRING) {
      j[c.first] = c.second.valueStr;

    } else if (c.second.type == VARIANT_INT64 ||
               c.second.type == VARIANT_INT32) {
      j[c.first] = c.second.valueInt64;

    } else if (c.second.type == VARIANT_DOUBLE ||
               c.second.type == VARIANT_FLOAT) {
      j[c.first] = c.second.valueDouble;
    }
  }

  std::string jsonFilename = "_" + sanitizeName(mRunningDirectory) +
                             std::to_string(long(this)) + "_config.json";
  if (mVerbose) {
    std::cout << "Writing json config: " << jsonFilename << std::endl;
  }
  {
    PushDirectory p(mRunningDirectory, mVerbose);
    std::ofstream of(jsonFilename, std::ofstream::out);
    if (of.good()) {
      of << j.dump(4);
      of.close();
      if (!of.good()) {
        std::cout << "Error writing json file." << std::endl;
        return "";
      }
    } else {
      std::cout << "Error writing json file." << std::endl;
      return "";
    }
  }
  return jsonFilename;
}

void ProcessorScript::parametersToConfig(nlohmann::json &j) {

  for (al::ParameterMeta *param : mParameters) {
    // TODO should we use full address or group + name?
    std::string name = param->getName();
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + "/" + name;
    }
    if ((strcmp(typeid(*param).name(), typeid(al::Parameter).name()) == 0)) {
      j[name] = dynamic_cast<al::Parameter *>(param)->get();
    } else if ((strcmp(typeid(*param).name(),
                       typeid(al::ParameterString).name()) == 0)) {
      j[name] = dynamic_cast<al::ParameterString *>(param)->get();
    } else if ((strcmp(typeid(*param).name(),
                       typeid(al::ParameterInt).name()) == 0)) {
      j[name] = dynamic_cast<al::ParameterInt *>(param)->get();
    }
  }
}

std::string ProcessorScript::makeCommandLine() {
  std::string commandLine = mScriptCommand + " ";
  for (auto &flag : configuration) {
    switch (flag.second.type) {
    case VARIANT_STRING:
      commandLine += flag.second.commandFlag + flag.second.valueStr + " ";

      break;
    case VARIANT_INT64:
    case VARIANT_INT32:
      commandLine += flag.second.commandFlag +
                     std::to_string(flag.second.valueInt64) + " ";
      break;
    case VARIANT_DOUBLE:
    case VARIANT_FLOAT:
      commandLine += flag.second.commandFlag +
                     std::to_string(flag.second.valueDouble) + " ";

      break;
    }
  }
  return commandLine;
}

bool ProcessorScript::runCommand(const std::string &command) {
  PushDirectory p(mRunningDirectory, mVerbose);

  if (mVerbose) {
    std::cout << "ProcessorScript command: " << command << std::endl;
  }
  std::array<char, 128> buffer{0};
  std::string output;
  // FIXME fork if running async
  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe)
    throw std::runtime_error("popen() failed!");
  while (!feof(pipe)) {
    if (fgets(buffer.data(), 128, pipe) != nullptr) {
      output += buffer.data();
      if (mVerbose) {
        std::cout << buffer.data() << std::endl;
      }
    }
  }

  int returnValue = 0;
  // TODO: Put back return value checking. Currently crashing
  //        int returnValue = -1;
  if (!ferror(pipe)) {
    returnValue = pclose(pipe);
  } else {
    returnValue = -1;
  }

  if (mVerbose) {
    std::cout << "Script result: " << returnValue << std::endl;
    std::cout << output << std::endl;
  }
  return returnValue == 0;
}

bool ProcessorScript::writeMeta() {

  nlohmann::json j;

  j["__tinc_metadata_version"] = DATASCRIPT_META_FORMAT_VERSION;
  j["__script"] = scriptFile(false);
  j["__script_modified"] = modified(scriptFile().c_str());
  j["__running_directory"] = getRunningDirectory();
  // TODO add support for multiple input and output files.
  j["__output_dir"] = getOutputDirectory();
  j["__output_name"] = outputFile(false);
  j["__input_dir"] = getInputDirectory();
  j["__input_modified"] = modified(inputFile().c_str());
  j["__input_name"] = inputFile(false);

  // TODO add date and other important information.

  for (auto option : configuration) {
    switch (option.second.type) {
    case VARIANT_STRING:
      j[option.first] = option.second.valueStr;
      break;
    case VARIANT_INT64:
    case VARIANT_INT32:
      j[option.first] = option.second.valueInt64;
      break;
    case VARIANT_DOUBLE:
    case VARIANT_FLOAT:
      j[option.first] = option.second.valueDouble;
      break;
    }
  }
  std::string jsonFilename = metaFilename();
  if (mVerbose) {
    std::cout << "Wrote cache in: " << metaFilename() << std::endl;
  }
  {
    PushDirectory p(mRunningDirectory, mVerbose);
    std::ofstream of(jsonFilename, std::ofstream::out);
    if (of.good()) {
      of << j.dump(4);
      of.close();
      if (!of.good()) {
        std::cout << "Error writing json file." << std::endl;
        return false;
      }
    } else {
      std::cout << "Error writing json file." << std::endl;
      return false;
    }
  }
  return true;
}

al_sec ProcessorScript::modified(const char *path) const {
  struct stat s;
  if (::stat(path, &s) == 0) {
    // const auto& t = s.st_mtim;
    // return t.tv_sec + t.tv_usec/1e9;
    return s.st_mtime;
  }
  return 0.;
}

bool ProcessorScript::needsRecompute() {
  std::ifstream metaFileStream;
  metaFileStream.open(metaFilename(), std::ofstream::in);

  if (metaFileStream.fail()) {
    if (mVerbose) {
      std::cout << "Failed to open metadata: Recomputing. " << metaFilename()
                << std::endl;
    }
    return true;
  }
  nlohmann::json metaData;
  try {
    metaData = nlohmann::json::parse(metaFileStream);
  } catch (...) {
    std::cout << "Error parsing: " << metaFilename() << std::endl;
    metaFileStream.close();
  }

  metaFileStream.close();
  if (!metaData.is_object()) {
    return true;
  }

  if (metaData["__tinc_metadata_version"] != DATASCRIPT_META_FORMAT_VERSION) {
    if (mVerbose) {
      std::cout << "Metadata format mismatch. Forcing recompute" << std::endl;
    }
    return true;
  }
  if (metaData["__script_modified"] != modified(scriptFile().c_str())) {
    return true;
  }
  if (al::File::exists(inputFile()) && inputFile(false).size() > 0) {
    if (metaData["__input_modified"] != modified(inputFile().c_str())) {
      return true;
    }
  }
  if (!al::File::exists(outputFile())) {
    return true;
  }

  return false;
}

std::string ProcessorScript::metaFilename() {
  std::string outPath = getOutputDirectory();
  std::string outName = outputFile(false);
  std::string metafilename =
      al::File::conformPathToOS(outPath) + outName + ".meta";
  return metafilename;
}
