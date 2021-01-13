#include "tinc/Processor.hpp"

#include "al/io/al_File.hpp"

#include <iostream>

// For PushDirectory
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

std::mutex PushDirectory::mDirectoryLock;

PushDirectory::PushDirectory(std::string directory, bool verbose)
    : mVerbose(verbose) {
  mDirectoryLock.lock();
  getcwd(previousDirectory, sizeof(previousDirectory));
  chdir(directory.c_str());
  if (mVerbose) {
    std::cout << "Pushing directory: " << directory << std::endl;
    char curDir[4096];
    getcwd(curDir, sizeof(curDir));
    std::cout << "now at: " << curDir << std::endl;
  }
}

PushDirectory::~PushDirectory() {
  chdir(previousDirectory);
  if (mVerbose) {
    std::cout << "Setting directory back to: " << previousDirectory
              << std::endl;
  }
  mDirectoryLock.unlock();
}

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
  //  setRunningDirectory(directory);
}

void Processor::setOutputDirectory(std::string outputDirectory) {
  mOutputDirectory = al::File::conformPathToOS(outputDirectory);
  std::replace(mOutputDirectory.begin(), mOutputDirectory.end(), '\\', '/');
  if (!al::File::isDirectory(mOutputDirectory)) {
    if (!al::Dir::make(mOutputDirectory)) {
      std::cout << "Unable to create output directory:" << mOutputDirectory
                << std::endl;
    }
  }
}

void Processor::setInputDirectory(std::string inputDirectory) {
  mInputDirectory = al::File::conformPathToOS(inputDirectory);
  std::replace(mInputDirectory.begin(), mInputDirectory.end(), '\\', '/');
  if (!al::File::isDirectory(mInputDirectory)) {
    std::cout
        << "Warning input directory for Processor doesn't exist. Creating."
        << std::endl;
    if (!al::Dir::make(mInputDirectory)) {
      std::cout << "Unable to create input directory:" << mOutputDirectory
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

Processor &Processor::registerDimension(ParameterSpaceDimension &dim) {
  auto *param = dim.parameterMeta();
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
