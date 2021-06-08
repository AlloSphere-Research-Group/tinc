
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

std::string ProcessorScript::getScriptFile(bool fullPath) {
  return mScriptName;
}

bool ProcessorScript::process(bool forceRecompute) {
  if (!enabled) {
    return true;
  }

  if (mVerbose) {
    std::cerr << "STARTING ProcessorScript: " << mId << std::endl;
  }
  if (mScriptName == "" && mScriptCommand == "") {
    std::cout << "ERROR: process() for '" << getId()
              << "' missing script name or script command." << std::endl;
    return false;
  }
  if (prepareFunction) {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    if (!prepareFunction()) {
      std::cerr << "ERROR preparing processor: " << getId() << std::endl;
      return false;
    }
  }

  callStartCallbacks();
  std::unique_lock<std::mutex> lk(mProcessingLock);
  std::string jsonFilename;
  if (mEnableJsonConfig) {
    jsonFilename = writeJsonConfig();
    if (jsonFilename.size() == 0) {
      return false;
    }
  }
  bool ok = true;
  if (needsRecompute() || forceRecompute) {
    std::string command = mScriptCommand + " \"" + mScriptName + "\" ";
    if (mEnableJsonConfig) {
      command += "\"" + jsonFilename + "\"";
    }
    ok = runCommand(command);
    if (ok) {
      if (mUseCache) {
        writeMeta();
      }
      if (mEnableJsonConfig) {
        readJsonConfig(jsonFilename);
      }
    }
  } else {
    if (mEnableJsonConfig) {
      readJsonConfig(jsonFilename);
    }
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

void ProcessorScript::enableJsonConfig(bool enable) {
  mEnableJsonConfig = enable;
}

std::string ProcessorScript::writeJsonConfig() {
  using json = nlohmann::json;
  json j;

  j["__tinc_metadata_version"] = PROCESSOR_META_FORMAT_VERSION;
  j["__output_dir"] = getOutputDirectory();
  j["__output_names"] = getOutputFileNames();
  j["__input_dir"] = getInputDirectory();
  j["__input_names"] = getInputFileNames();
  j["__verbose"] = mVerbose;

  parametersToConfig(j);

  for (auto c : configuration) {
    if (c.second.type() == al::VariantType::VARIANT_STRING) {
      j[c.first] = c.second.get<std::string>();

    } else if (c.second.type() == al::VariantType::VARIANT_INT64) {
      j[c.first] = c.second.get<int64_t>();

    } else if (c.second.type() == al::VariantType::VARIANT_INT32) {
      j[c.first] = c.second.get<int32_t>();

    } else if (c.second.type() == al::VariantType::VARIANT_DOUBLE) {
      j[c.first] = c.second.get<double>();
    } else if (c.second.type() == al::VariantType::VARIANT_FLOAT) {
      j[c.first] = c.second.get<float>();
    }
  }

  std::string jsonFilename =
      "_" + mId + std::to_string(long(this)) + "_config.json";
  if (mVerbose) {
    std::cout << "Writing json config: " << jsonFilename << std::endl;
  }
  std::ofstream of;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    of.open(jsonFilename, std::ofstream::out);
  }
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
  return jsonFilename;
}

bool ProcessorScript::readJsonConfig(std::string filename) {
  using json = nlohmann::json;
  json j;
  std::ifstream f;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    f.open(filename);
  }
  if (!f.good()) {
    std::cerr << __FILE__ << "Error: can't open json config file: " << filename
              << std::endl;
    return false;
  }

  f >> j;
  try {
    if (j["__tinc_metadata_version"].get<int>() ==
        PROCESSOR_META_FORMAT_VERSION) {
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
  // TODO ML complete for all types. Done.
  for (auto &flag : configuration) {
    switch (flag.second.type()) {
    case al::VariantType::VARIANT_STRING:
      commandLine += flag.second.get<std::string>() + " ";
      break;
    case al::VariantType::VARIANT_INT64:
      commandLine += std::to_string(flag.second.get<int64_t>()) + " ";
      break;
    case al::VariantType::VARIANT_UINT64:
      commandLine += std::to_string(flag.second.get<uint64_t>()) + " ";
      break;
    case al::VariantType::VARIANT_INT32:
      commandLine += std::to_string(flag.second.get<int32_t>()) + " ";
      break;
    case al::VariantType::VARIANT_UINT32:
      commandLine += std::to_string(flag.second.get<uint32_t>()) + " ";
      break;
    case al::VariantType::VARIANT_INT16:
      commandLine += std::to_string(flag.second.get<int16_t>()) + " ";
      break;
    case al::VariantType::VARIANT_UINT16:
      commandLine += std::to_string(flag.second.get<uint16_t>()) + " ";
      break;
    case al::VariantType::VARIANT_INT8:
      commandLine += std::to_string(flag.second.get<int8_t>()) + " ";
      break;
    case al::VariantType::VARIANT_UINT8:
      commandLine += std::to_string(flag.second.get<uint8_t>()) + " ";
      break;
    case al::VariantType::VARIANT_DOUBLE:
      commandLine += std::to_string(flag.second.get<double>()) + " ";
      break;
    case al::VariantType::VARIANT_FLOAT:
      commandLine += std::to_string(flag.second.get<float>()) + " ";
    case al::VariantType::VARIANT_CHAR:
      commandLine += std::to_string(flag.second.get<char>()) + " ";
      break;
    case al::VariantType::VARIANT_BOOL:
      commandLine += std::to_string(flag.second.get<bool>()) + " ";
      break;
    case al::VariantType::VARIANT_NONE:
      break;
    }
  }

  return commandLine;
}

bool ProcessorScript::runCommand(const std::string &command) {

  if (mVerbose) {
    std::cout << "ProcessorScript " << mId << " command: " << command
              << std::endl;
  }
  std::array<char, 128> buffer{0};
  std::string output;

  // FIXME fork if running async
  FILE *pipe;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    pipe = popen(command.c_str(), "r");
  }
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

  j["__tinc_metadata_version"] = PROCESSOR_META_FORMAT_VERSION;
  j["__script"] = getScriptFile(false);
  j["__script_modified"] = al::File::modificationTime(getScriptFile().c_str());
  j["__running_directory"] = getRunningDirectory();
  // TODO add support for multiple input and output files.
  j["__output_dir"] = getOutputDirectory();
  j["__output_names"] = getOutputFileNames();
  j["__input_dir"] = getInputDirectory();
  std::vector<al_sec> modifiedTimes;
  for (auto file : getInputFileNames()) {
    modifiedTimes.push_back(
        al::File::modificationTime((getInputDirectory() + file).c_str()));
  }
  j["__input_modified"] = modifiedTimes;
  j["__input_names"] = getInputFileNames();

  // TODO add date and other important information.

  // TODO ML support all types. Done
  for (auto option : configuration) {
    switch (option.second.type()) {
    case al::VariantType::VARIANT_STRING:
      j[option.first] = option.second.get<std::string>();
      break;
    case al::VariantType::VARIANT_INT64:
      j[option.first] = option.second.get<int64_t>();
      break;
    case al::VariantType::VARIANT_UINT64:
      j[option.first] = option.second.get<uint64_t>();
      break;
    case al::VariantType::VARIANT_INT32:
      j[option.first] = option.second.get<int32_t>();
      break;
    case al::VariantType::VARIANT_UINT32:
      j[option.first] = option.second.get<uint32_t>();
      break;
    case al::VariantType::VARIANT_INT16:
      j[option.first] = option.second.get<int16_t>();
      break;
    case al::VariantType::VARIANT_UINT16:
      j[option.first] = option.second.get<uint16_t>();
      break;
    case al::VariantType::VARIANT_INT8:
      j[option.first] = option.second.get<int8_t>();
      break;
    case al::VariantType::VARIANT_UINT8:
      j[option.first] = option.second.get<uint8_t>();
      break;
    case al::VariantType::VARIANT_DOUBLE:
      j[option.first] = option.second.get<double>();
      break;
    case al::VariantType::VARIANT_FLOAT:
      j[option.first] = option.second.get<float>();
      break;
    case al::VariantType::VARIANT_CHAR:
      j[option.first] = option.second.get<char>();
      break;
    case al::VariantType::VARIANT_BOOL:
      j[option.first] = option.second.get<bool>();
      break;
    case al::VariantType::VARIANT_NONE:
      break;
    }
  }
  std::string jsonFilename = metaFilename();
  if (mVerbose) {
    std::cout << "Wrote cache in: " << metaFilename() << std::endl;
  }

  std::ofstream of;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    of.open(jsonFilename, std::ofstream::out);
  }
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

  return true;
}

bool ProcessorScript::needsRecompute() {

  if (Processor::needsRecompute()) {
    return true;
  }
  std::ifstream metaFileStream;
  {
    al::PushDirectory p(mRunningDirectory, mVerbose);
    metaFileStream.open(metaFilename(), std::ofstream::in);
  }

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
  if (metaData["__script_modified"] !=
      al::File::modificationTime(getScriptFile().c_str())) {
    return true;
  }

  return false;
}
