
#include "tinc/ProcessorScript.hpp"

#include "al/io/al_File.hpp"

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

std::string ProcessorScript::getScriptFile(bool fullPath) {
  return mScriptName;
}

bool ProcessorScript::process(bool forceRecompute) {
  if (!enabled) {
    return true;
  }
  if (prepareFunction && !prepareFunction()) {
    std::cerr << "ERROR preparing processor: " << getId() << std::endl;
    return false;
  }
  if (mScriptName == "" && mScriptCommand == "") {
    std::cout << "ERROR: process() for '" << getId()
              << "' missing script name or script command." << std::endl;
    return false;
  }
  callStartCallbacks();
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
      readJsonConfig(jsonFilename);
    }
  } else {
    readJsonConfig(jsonFilename);
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
  std::replace(output_name.begin(), output_name.end(), '<', '_');
  std::replace(output_name.begin(), output_name.end(), '>', '_');
  std::replace(output_name.begin(), output_name.end(), '(', '_');
  std::replace(output_name.begin(), output_name.end(), ')', '_');
  return output_name;
}

void ProcessorScript::useCache(bool use) { mUseCache = use; }

std::string ProcessorScript::writeJsonConfig() {
  using json = nlohmann::json;
  json j;

  j["__tinc_metadata_version"] = DATASCRIPT_META_FORMAT_VERSION;
  j["__output_dir"] = getOutputDirectory();
  j["__output_names"] = getOutputFileNames();
  j["__input_dir"] = getInputDirectory();
  j["__input_names"] = getInputFileNames();
  j["__verbose"] = mVerbose;

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
    al::PushDirectory p(mRunningDirectory, mVerbose);
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

bool ProcessorScript::readJsonConfig(std::string filename) {
  using json = nlohmann::json;
  json j;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    std::ifstream f(filename);
    if (!f.good()) {
      std::cerr << __FILE__
                << "Error: can't open json config file: " << filename
                << std::endl;
      return false;
    }

    f >> j;
  }
  try {
    if (j["__tinc_metadata_version"].get<int>() ==
        DATASCRIPT_META_FORMAT_VERSION) {
      setOutputDirectory(j["__output_dir"].get<std::string>());
      setOutputFileNames(j["__output_names"].get<std::vector<std::string>>());
      setInputDirectory(j["__input_dir"].get<std::string>());
      setInputFileNames(j["__input_names"].get<std::vector<std::string>>());
      // We can ignore ["__verbose"] on read
    } else {
      std::cerr << "ERROR: Unexpected __tinc_metadata_version in json config"
                << std::endl;
    }
  } catch (std::exception &e) {
    std::cerr << "ERROR parsing json config file. Changes in python not "
                 "completely applied."
              << std::endl;
    return false;
  }
  if (mVerbose) {
    std::cout << "Read json config: " << filename << std::endl;
  }
  return true;
}

void ProcessorScript::parametersToConfig(nlohmann::json &j) {

  for (al::ParameterMeta *param : mParameters) {
    // TODO should we use full address or group + name?
    std::string name = param->getName();
    if (param->getGroup().size() > 0) {
      name = param->getGroup() + "/" + name;
    }
    if (auto p = dynamic_cast<al::Parameter *>(param)) {
      j[name] = p->get();
    } else if (auto p = dynamic_cast<al::ParameterString *>(param)) {
      j[name] = p->get();
    } else if (auto p = dynamic_cast<al::ParameterInt *>(param)) {
      j[name] = p->get();
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
  al::PushDirectory p(mRunningDirectory, mVerbose);

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
  j["__script"] = getScriptFile(false);
  j["__script_modified"] = modified(getScriptFile().c_str());
  j["__running_directory"] = getRunningDirectory();
  // TODO add support for multiple input and output files.
  j["__output_dir"] = getOutputDirectory();
  j["__output_names"] = getOutputFileNames();
  j["__input_dir"] = getInputDirectory();
  std::vector<al_sec> modifiedTimes;
  for (auto file : getInputFileNames()) {
    modifiedTimes.push_back(modified((getInputDirectory() + file).c_str()));
  }
  j["__input_modified"] = modifiedTimes;
  j["__input_names"] = getInputFileNames();

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
    al::PushDirectory p(mRunningDirectory, mVerbose);
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
  if (!mUseCache) {
    return true;
  }
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
  if (metaData["__script_modified"] != modified(getScriptFile().c_str())) {
    return true;
  }
  if (metaData["__input_modified"].size() != mInputFileNames.size()
      || !metaData["__input_modified"].is_array()) {
    return true;
  }
  for (int i = 0; i < metaData["__input_modified"].size(); i++) {
    if (metaData["__input_modified"][i] !=
        modified((getInputDirectory() + mInputFileNames[i]).c_str())) {
      return true;
    }
  }

  for (auto file : getOutputFileNames()) {
    if (!al::File::exists(getOutputDirectory() + file)) {
      return true;
    }
  }

  return false;
}

std::string ProcessorScript::metaFilename() {
  std::string outPath = getOutputDirectory();
  std::string outName = "out.meta";
  if (mOutputFileNames.size() > 0) {
    outName = getOutputFileNames()[0];
  }
  std::string metafilename =
      al::File::conformPathToOS(outPath) + outName + ".meta";
  return metafilename;
}
