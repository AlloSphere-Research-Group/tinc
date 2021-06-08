#include "tinc/Processor.hpp"

#include "al/io/al_File.hpp"

#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>

using namespace tinc;

// --------------------------------------------------

bool Processor::isRunning() {
  if (mProcessLock.try_lock()) {
    mProcessLock.unlock();
    return false;
  }
  return true;
}

void Processor::setDataDirectory(std::string directory) {
  setOutputDirectory(directory);
  setInputDirectory(directory);
}

void Processor::setOutputDirectory(std::string outputDirectory) {
  mOutputDirectory =
      al::File::conformDirectory(al::File::conformPathToOS(outputDirectory));
  std::replace(mOutputDirectory.begin(), mOutputDirectory.end(), '\\', '/');
  if (mOutputDirectory.size() > 0 && !al::File::isDirectory(mOutputDirectory)) {
    if (!al::Dir::make(mOutputDirectory)) {
      std::cout << "Unable to create output directory:" << mOutputDirectory
                << std::endl;
    }
  }
}

void Processor::setInputDirectory(std::string inputDirectory) {
  mInputDirectory =
      al::File::conformDirectory(al::File::conformPathToOS(inputDirectory));
  //  std::replace(mInputDirectory.begin(), mInputDirectory.end(), '\\', '/');
  if (mInputDirectory.size() > 0 && !al::File::isDirectory(mInputDirectory)) {
    std::cout
        << "Warning input directory for Processor doesn't exist. Creating."
        << std::endl;
    if (!al::Dir::make(mInputDirectory)) {
      std::cout << "Unable to create input directory:" << mInputDirectory
                << std::endl;
    }
  }
}

void Processor::setRunningDirectory(std::string directory) {
  mRunningDirectory = al::File::conformPathToOS(directory);
  std::replace(mRunningDirectory.begin(), mRunningDirectory.end(), '\\', '/');
  if (!al::File::exists(mRunningDirectory)) {
    if (!al::Dir::make(mRunningDirectory)) {
      std::cout << "Error creating directory: " << mRunningDirectory
                << std::endl;
    }
  }
}

void Processor::registerStartCallback(std::function<void()> func) {
  mStartCallbacks.push_back(func);
}

void Processor::registerDoneCallback(std::function<void(bool)> func) {
  mDoneCallbacks.push_back(func);
}

Processor &Processor::registerDimension(ParameterSpaceDimension &dim) {
  auto *param = dim.getParameterMeta();
  if (auto *p = dynamic_cast<al::Parameter *>(param)) {
    return registerParameter(*p);
  } /*else if (auto *p =dynamic_cast<al::ParameterBool *>(param)) {
      return registerParameter(*p);
    }  */
  else if (auto *p = dynamic_cast<al::ParameterInt *>(param)) {
    return registerParameter(*p);
  } else if (auto *p = dynamic_cast<al::ParameterString *>(param)) {
    return registerParameter(*p);
  } else {
    std::cerr << __FUNCTION__ << "ERROR: Unsupported dimension type."
              << std::endl;
  }
  return *this;
}

void Processor::setOutputFileNames(std::vector<std::string> outputFiles) {
  mOutputFileNames.clear();
  for (auto fileName : outputFiles) {
    auto name = al::File::conformPathToOS(fileName);
    // FIXME this is not being used everywhere it should be....
    mOutputFileNames.push_back(name);
  }
}

std::vector<std::string> Processor::getOutputFileNames() {
  return mOutputFileNames;
}

void Processor::setInputFileNames(std::vector<std::string> inputFiles) {
  mInputFileNames.clear();
  for (auto fileName : inputFiles) {
    auto name = al::File::conformPathToOS(fileName);
    // FIXME this is not being used everywhere it should be....
    mInputFileNames.push_back(name);
  }
}

std::vector<std::string> Processor::getInputFileNames() {
  return mInputFileNames;
}

void Processor::callStartCallbacks() {
  al::PushDirectory dir(mRunningDirectory, mVerbose);
  for (auto cb : mStartCallbacks) {
    cb();
  }
}

void Processor::callDoneCallbacks(bool result) {
  al::PushDirectory p(mRunningDirectory, mVerbose);
  for (auto cb : mDoneCallbacks) {
    cb(result);
  }
}

bool Processor::writeMeta() {

  nlohmann::json j;

  j["__tinc_metadata_version"] = PROCESSOR_META_FORMAT_VERSION;
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

  // TODO ML verify support for the rest of the types. Done.
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

bool Processor::needsRecompute() {
  if (!mUseCache) {
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

  metaFileStream.close();
  if (!metaData.is_object()) {
    return true;
  }

  if (metaData["__tinc_metadata_version"] != PROCESSOR_META_FORMAT_VERSION) {
    if (mVerbose) {
      std::cout << "Metadata format mismatch. Forcing recompute" << std::endl;
    }
    return true;
  }
  if (metaData["__input_modified"].size() != mInputFileNames.size() ||
      !metaData["__input_modified"].is_array()) {
    return true;
  }
  for (int i = 0; i < metaData["__input_modified"].size(); i++) {
    if (metaData["__input_modified"][i] !=
        al::File::modificationTime(
            (getInputDirectory() + mInputFileNames[i]).c_str())) {
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

std::string Processor::metaFilename() {
  std::string outPath = getOutputDirectory();
  std::string outName = "out.meta";
  if (mOutputFileNames.size() > 0) {
    outName = getOutputFileNames()[0];
  }
  std::string metafilename =
      al::File::conformPathToOS(outPath) + outName + ".meta";
  return metafilename;
}
