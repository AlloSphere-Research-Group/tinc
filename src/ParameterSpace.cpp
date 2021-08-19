// FOR strncpy_s
#define __STDC_WANT_LIB_EXT1__ 1

#include "tinc/ParameterSpace.hpp"
#include "tinc/ProcessorGraph.hpp"

#include "al/io/al_File.hpp"

#ifdef TINC_HAS_HDF5
#include <netcdf.h>
#endif

#if defined(AL_OSX) || defined(AL_LINUX) || defined(AL_EMSCRIPTEN)
#include <sys/stat.h> // For non C++17 file time functions
#include <unistd.h>
#else

#endif

#include <chrono>
#include <ctime>
#ifdef TINC_CPP_17
#include <filesystem>
#else
#include "al/io/al_File.hpp"
#endif
#include <iomanip>
#include <iostream>

#include "picosha2.h" // SHA256 hash generator

#include <tinc/ParameterSpace.hpp>

using namespace tinc;

ParameterSpace::~ParameterSpace() { stopSweep(); }

ParameterSpaceDimension *ParameterSpace::getDimension(std::string name,
                                                      std::string group) {
  std::unique_lock<std::mutex> lk(mDimensionsLock);
  if (parameterNameMap.find(name) != parameterNameMap.end()) {
    name = parameterNameMap[name];
  }
  for (auto &psd : mDimensions) {
    if (psd->getName() == name && psd->getGroup() == group) {
      return psd.get();
    } else if (group == "" && psd->getFullAddress() == name) {
      return psd.get();
    }
  }
  return nullptr;
}

ParameterSpaceDimension *
ParameterSpace::newDimension(std::string name,
                             ParameterSpaceDimension::RepresentationType type,
                             al::VariantType datatype, std::string group) {
  // FIXME check if dimension exists already and return existing one
  auto newDim =
      std::make_shared<ParameterSpaceDimension>(name, group, datatype);
  newDim->mRepresentationType = type;

  registerDimension(newDim);

  return newDim.get();
}

ParameterSpaceDimension *ParameterSpace::registerDimension(
    std::shared_ptr<ParameterSpaceDimension> &dimension, al::Socket *src) {
  std::unique_lock<std::mutex> lk(mDimensionsLock);

  for (auto &dim : mDimensions) {
    if (dim->getName() == dimension->getName() &&
        dim->getGroup() == dimension->getGroup()) {
      if (dim->mSpaceValues.getDataType() ==
          dimension->mSpaceValues.getDataType()) {
        dim->mSpaceValues.clear();
        dim->mSpaceValues.append(dimension->mSpaceValues.getValuesPtr(),
                                 dimension->mSpaceValues.size());
        dim->mSpaceValues.setIds(dimension->mSpaceValues.getIds());
        dim->mRepresentationType = dimension->getSpaceRepresentationType();

        lk.unlock();
        onDimensionRegister(dim.get(), this, src);
        return dim.get();
      } else {
        std::cerr << "WARNING: Dimension datatype change." << std::endl;
      }
    }
  }

  if (al::ParameterBool *p =
          dynamic_cast<al::ParameterBool *>(dimension->getParameterMeta())) {
    auto &param = *p;
    param.registerChangeCallback([&dimension, &param, this](float value) {
      float oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::Parameter *p =
                 dynamic_cast<al::Parameter *>(dimension->getParameterMeta())) {
    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](float value) {
      float oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](int32_t value) {
      int32_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);
  } else if (al::ParameterInt64 *p = dynamic_cast<al::ParameterInt64 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](int64_t value) {
      int64_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterInt16 *p = dynamic_cast<al::ParameterInt16 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](int16_t value) {
      int16_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterInt8 *p = dynamic_cast<al::ParameterInt8 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](int8_t value) {
      int8_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterUInt64 *p = dynamic_cast<al::ParameterUInt64 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](uint64_t value) {
      uint64_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterUInt32 *p = dynamic_cast<al::ParameterUInt32 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](uint32_t value) {
      uint32_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterUInt16 *p = dynamic_cast<al::ParameterUInt16 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](uint16_t value) {
      uint16_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterUInt8 *p = dynamic_cast<al::ParameterUInt8 *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](uint8_t value) {
      uint8_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  } else if (al::ParameterDouble *p = dynamic_cast<al::ParameterDouble *>(
                 dimension->getParameterMeta())) {

    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](double value) {
      double oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });

    mDimensions.push_back(dimension);
    lk.unlock();
    onDimensionRegister(dimension.get(), this, src);

  }

  else {
    // FIXME ML implement for all parameter types. Done.
    std::cerr << "Support for parameter type not implemented in dimension "
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
  return dimension.get();
}

void ParameterSpace::removeDimension(std::string name, std::string group,
                                     bool invoked, al::Socket *src) {
  std::unique_lock<std::mutex> lk(mDimensionsLock);

  auto it = mDimensions.begin();
  while (it < mDimensions.end()) {
    if (((*it)->getName() == name && (*it)->getGroup() == group) ||
        (group == "" && (*it)->getFullAddress() == name)) {
      onDimensionRemove(it->get(), this, invoked, src);
      it = mDimensions.erase(it);
      break;
    } else {
      ++it;
    }
  }
}

std::vector<ParameterSpaceDimension *> ParameterSpace::getDimensions() {
  std::vector<ParameterSpaceDimension *> dims;
  std::unique_lock<std::mutex> lk(mDimensionsLock);
  for (auto &psd : mDimensions) {
    dims.push_back(psd.get());
  }
  return dims;
}

std::vector<std::string>
ParameterSpace::runningPaths(std::vector<std::string> fixedDimensions) {
  std::vector<std::string> paths;

  std::map<std::string, size_t> currentIndices;

  std::vector<std::string> dimNames = getDimensionNames();
  for (auto &dimensionName : dimNames) {
    auto dimension = getDimension(dimensionName);
    if (isFilesystemDimension(dimension->getName()) &&
        (std::find(fixedDimensions.begin(), fixedDimensions.end(),
                   dimension->getName()) == fixedDimensions.end())) {
      currentIndices[dimension->getName()] = 0;
    }
  }
  bool done = false;
  while (!done) {
    done = true;
    auto path = al::File::conformDirectory(
        al::File::conformPathToOS(mRootPath) +
        generateRelativeRunPath(currentIndices, this));
    if (path.size() > 0 &&
        std::find(paths.begin(), paths.end(), path) == paths.end()) {
      paths.push_back(path);
    }
    done = incrementIndices(currentIndices);
  }
  return paths;
}

std::string ParameterSpace::getCurrentRelativeRunPath() {
  std::map<std::string, size_t> indices;
  {
    std::unique_lock<std::mutex> lk(mDimensionsLock);
    for (auto &psd : mDimensions) {
      // if (psd->isFilesystemDimension()) {
      indices[psd->getName()] = psd->getCurrentIndex();
      // }
    }
  }
  return generateRelativeRunPath(indices, this);
}

std::string ParameterSpace::getCommonId(std::vector<std::string> dimNames,
                                        std::map<std::string, size_t> indices) {
  if (dimNames.size() == 0) {
    dimNames = getDimensionNames();
  }
  std::vector<ParameterSpaceDimension *> dimensions;
  for (auto &dimName : dimNames) {
    dimensions.push_back(getDimension(dimName));
  }
  std::map<ParameterSpaceDimension *, size_t> indecesOverride;
  for (auto *dim : getDimensions()) {
    auto found = indices.find(dim->getName());
    if (found != indices.end()) {
      indecesOverride[dim] = found->second;
    }
  }
  return getCommonId(dimensions, indecesOverride);
}

std::string ParameterSpace::getCommonId(
    std::vector<ParameterSpaceDimension *> dimensions,
    std::map<ParameterSpaceDimension *, size_t> indices) {
  std::vector<ParameterSpaceDimension *> relevantDimensions;
  for (auto *dim : dimensions) {
    if (dim && dim->getCurrentIds().size() > 1) {
      relevantDimensions.push_back(dim);
    }
  }
  if (relevantDimensions.size() > 1) {
    auto dimIt = relevantDimensions.begin();
    auto ids = (*dimIt)->getCurrentIds();
    if (indices.find(*dimIt) != indices.end()) {
      ids = (*dimIt)->getIdsForValue((*dimIt)->at(indices[(*dimIt)]));
    }
    dimIt++;
    while (dimIt != relevantDimensions.end() && ids.size() > 1) {
      auto dimIds = (*dimIt)->getCurrentIds();
      if (indices.find(*dimIt) != indices.end()) {
        dimIds = (*dimIt)->getIdsForValue((*dimIt)->at(indices[(*dimIt)]));
      }
      std::vector<std::string> idsToRemove;
      for (auto &id : ids) {
        auto pos = std::find(dimIds.begin(), dimIds.end(), id);
        if (pos == dimIds.end()) {
          idsToRemove.push_back(id);
        }
      }
      for (auto &id : idsToRemove) {
        ids.erase(std::find(ids.begin(), ids.end(), id));
      }
      dimIt++;
    }
    if (ids.size() == 1) {
      return ids[0];
    } else {
      std::cerr << __FUNCTION__ << " Could not find common id." << std::endl;
    }

  } else {
    std::cerr << __FUNCTION__
              << ": Less than two valid dimensions provided. No id returned"
              << std::endl;
  }
  return std::string();
}

std::vector<std::string> ParameterSpace::getDimensionNames() {
  std::unique_lock<std::mutex> lk(mDimensionsLock);
  std::vector<std::string> dimensionNames;
  for (auto &dim : mDimensions) {
    dimensionNames.push_back(dim->getName());
  }
  return dimensionNames;
}

bool ParameterSpace::isFilesystemDimension(std::string dimensionName) {
  auto dim = getDimension(dimensionName);
  if (dim && dim->size() > 1) {
    // This should be enough of a check, or should we check all possible
    // values?
    std::map<std::string, size_t> indices;
    indices[dim->getName()] = {0};
    auto path0 = generateRelativeRunPath(indices, this);
    indices[dim->getName()] = {dim->getSpaceStride()};
    auto path1 = generateRelativeRunPath(indices, this);
    if (path0 != path1) {
      return true;
    }
  }
  // TODO what if the template contains the dimension name,
  return false;
}

void ParameterSpace::clear() {
  std::unique_lock<std::mutex> lk(mDimensionsLock);

  auto it = mDimensions.begin();
  while (it < mDimensions.end()) {
    onDimensionRemove(it->get(), this, false, nullptr);
    it = mDimensions.erase(it);
  }

  mSpecialDirs.clear();
}

bool ParameterSpace::incrementIndices(
    std::map<std::string, size_t> &currentIndices) {

  for (auto &dimensionIndex : currentIndices) {
    auto dimension = getDimension(dimensionIndex.first);
    dimensionIndex.second += dimension->getSpaceStride();
    if (dimensionIndex.second >= dimension->size()) {
      dimensionIndex.second = 0;
    } else {
      return false;
    }
  }
  return true;
}

bool ParameterSpace::runProcess(
    Processor &processor, const std::map<std::string, al::VariantValue> &args,
    const std::map<std::string, al::VariantValue> &dependencies,
    bool recompute) {

  if (auto *procChain = dynamic_cast<ProcessorGraph *>(&processor)) {
    // TODO enable async processing of processor chain
    bool ret = true;
    for (auto *childProc : procChain->getProcessors()) {
      bool childRet = runProcess(*childProc, args, dependencies, recompute);
      ret &= childRet;
      if (!childRet && !childProc->ignoreFail) {
        break;
      }
    }
    return ret;
  } else {
    auto path = getCurrentRelativeRunPath();
    if (path.size() > 0) {
      // TODO allow fine grained options of what directory to set
      processor.setRunningDirectory(getRootPath() + path);
    }
    // First set the current values in the parameter space
    {
      std::unique_lock<std::mutex> lk(mDimensionsLock);
      for (auto dim : mDimensions) {
        if (args.find(dim->getName()) == args.end()) {
          if (dim->mRepresentationType == ParameterSpaceDimension::VALUE) {
            processor.configuration[dim->getName()] = dim->getCurrentValue();
          } else if (dim->mRepresentationType == ParameterSpaceDimension::ID) {
            processor.configuration[dim->getName()] = dim->getCurrentId();
          } else if (dim->mRepresentationType ==
                     ParameterSpaceDimension::INDEX) {
            assert(dim->getCurrentIndex() <
                   std::numeric_limits<int64_t>::max());
            processor.configuration[dim->getName()] =
                (uint64_t)dim->getCurrentIndex();
          }
        }
      }
    }
    // Then set dependencies. Dependencies and arguments override values in the
    // parameter space. Although that is not meant to be their use.
    for (auto &dep : dependencies) {
      processor.configuration[dep.first] = dep.second;
    }
    // Then set the provided arguments
    for (auto &arg : args) {
      processor.configuration[arg.first] = arg.second;
    }
    return executeProcess(processor, recompute);
  }
}

void ParameterSpace::sweep(Processor &processor,
                           std::vector<std::string> dimensionNames_,
                           std::map<std::string, al::VariantValue> dependencies,
                           bool recompute) {
  uint64_t sweepCount = 0;
  uint64_t sweepTotal = 1;
  mSweepRunning = true;
  if (dimensionNames_.size() == 0) {
    dimensionNames_ = getDimensionNames();
  }
  for (auto &dimensionName : dimensionNames_) {
    auto dim = getDimension(dimensionName);
    if (dim) {
      if (dim->size() > 0) {
        sweepTotal *= dim->size();
      }
    } else {
      std::cerr << __FUNCTION__
                << " ERROR: dimension not found: " << dimensionName
                << std::endl;
    }
  }
  for (auto dep : processor.getDependencies()) {
    if (dependencies.find(dep->getName()) != dependencies.end()) {
      // Don't override dependencies provided on command line
      continue;
    }
    std::vector<al::VariantValue> fields;
    dep->getParameterMeta()->getFields(fields);
    if (fields.size() == 1) {
      dependencies[dep->getName()] = fields[0];
    } else {
      std::cerr << __FILE__ << ":" << __LINE__
                << " ERROR: parameters with multiple values not supported"
                << std::endl;
    }
  }

  std::map<std::string, size_t> previousIndices;
  for (auto &dimName : dimensionNames_) {
    auto dim = getDimension(dimName);
    if (dim) {
      previousIndices[dimName] = dim->getCurrentIndex();
      dim->setCurrentIndex(0);
    }
  }

  while (mSweepRunning) {
    std::map<std::string, al::VariantValue> args;
    {
      std::unique_lock<std::mutex> lk(mDimensionsLock);
      for (auto &dim : mDimensions) {
        if (dim->mRepresentationType == ParameterSpaceDimension::VALUE) {
          args[dim->getName()] = dim->getCurrentValue();
        } else if (dim->mRepresentationType == ParameterSpaceDimension::ID) {
          args[dim->getName()] = dim->getCurrentId();
        } else if (dim->mRepresentationType == ParameterSpaceDimension::INDEX) {
          assert(dim->getCurrentIndex() < std::numeric_limits<int64_t>::max());
          args[dim->getName()] = (uint64_t)dim->getCurrentIndex();
        }
      }
    }
    for (auto &arg : args) {
      processor.configuration[arg.first] = arg.second;
    }
    // Dependencies override values from the parameter space
    for (auto &dep : dependencies) {
      processor.configuration[dep.first] = dep.second;
    }
    auto path =
        al::File::conformDirectory(mRootPath) + getCurrentRelativeRunPath();
    if (path.size() > 0) {
      // TODO allow fine grained options of what directory to set
      processor.setRunningDirectory(path);
    }
    sweepCount++;
    if (!executeProcess(processor, recompute) && !processor.ignoreFail) {
      std::cerr << "Processor failed in parameter sweep. Aborting" << std::endl;
      break;
    } else {
      if (onSweepProcess) {
        onSweepProcess(sweepCount / (double)sweepTotal);
      }
    }

    auto currentDimension = dimensionNames_.begin();
    while (currentDimension != dimensionNames_.end()) {
      auto dim = getDimension(*currentDimension);
      auto stride = dim->getSpaceStride();
      if (stride == 0 || (dim->getCurrentIndex() == dim->size() - stride)) {
        dim->setCurrentIndex(0);
      } else {
        dim->stepIncrement();
        break;
      }
      currentDimension++;
    }
    if (currentDimension == dimensionNames_.end()) {
      break;
    }
  }
  // Put back previous value
  for (auto &previousIndex : previousIndices) {
    if (previousIndex.second != SIZE_MAX) {
      getDimension(previousIndex.first)->setCurrentIndex(previousIndex.second);
    }
  }
  mSweepRunning = false;
}

void ParameterSpace::sweepAsync(
    Processor &processor, std::vector<std::string> dimensions,
    std::map<std::string, al::VariantValue> dependencies, bool recompute,
    int numThreads) {
  if (mAsyncProcessingThreads.size() > 0 || mAsyncPSCopy) {
    stopSweep();
  }
  mAsyncPSCopy = std::make_shared<ParameterSpace>();
  {
    std::unique_lock<std::mutex> lk(mDimensionsLock);
    for (auto &dim : ParameterSpace::mDimensions) {
      auto dimCopy = dim->deepCopy();
      mAsyncPSCopy->registerDimension(dimCopy);
    }
    mAsyncPSCopy->onSweepProcess = onSweepProcess;
    mAsyncPSCopy->onValueChange = onValueChange;
    mAsyncPSCopy->generateRelativeRunPath = generateRelativeRunPath;
    mAsyncPSCopy->mCurrentPathTemplate = mCurrentPathTemplate;
  }
  for (int i = 0; i < numThreads; i++) {
    // TODO slice the parameter space to split among threads
    mAsyncProcessingThreads.emplace_back(
        std::make_shared<std::thread>([=, &processor]() {
          mAsyncPSCopy->sweep(processor, dimensions, dependencies, recompute);
        }));
  }
}

bool ParameterSpace::createDataDirectories() {
  for (auto &path : runningPaths()) {
    if (!al::File::isDirectory(path)) {
      if (!al::Dir::make(path)) {
        return false;
      }
    }
  }
  return true;
}

bool ParameterSpace::cleanDataDirectories() {
  return removeDataDirectories() && createDataDirectories();
}

bool ParameterSpace::removeDataDirectories() {
  for (auto &path : runningPaths()) {
    auto fullpath = getRootPath() + path;
    if (al::File::isDirectory(fullpath)) {
      if (!al::Dir::removeRecursively(fullpath)) {
        return false;
      }
    }
  }
  return true;
  ;
}

void ParameterSpace::stopSweep() {
  mSweepRunning = false;
  if (mAsyncPSCopy) {
    mAsyncPSCopy->stopSweep();
  }
  for (auto &thread : mAsyncProcessingThreads) {
    thread->join();
  }
  mAsyncProcessingThreads.clear();
  mAsyncPSCopy = nullptr;
}

bool readNetCDFValues(int grpid, ParameterSpaceDimension *pdim) {

#ifdef TINC_HAS_HDF5
  int retval;
  int varid;
  nc_type xtypep;
  int ndimsp;
  int dimidsp[32];
  size_t lenp;
  int *nattsp = nullptr;
  if ((retval = nc_inq_varid(grpid, "values", &varid))) {
    return false;
  }
  if ((retval = nc_inq_var(grpid, varid, nullptr, &xtypep, &ndimsp, dimidsp,
                           nattsp))) {
    return false;
  }
  if ((retval = nc_inq_dimlen(grpid, dimidsp[0], &lenp))) {
    return false;
  }
  // TODO cover all supported cases and report errors.
  if (xtypep == NC_FLOAT) {
    std::vector<float> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->setSpaceValues(data.data(), data.size());
  } else if (xtypep == NC_INT) {

    std::vector<int32_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->setSpaceValues(data.data(), data.size());
  } else if (xtypep == NC_UBYTE) {

    std::vector<uint8_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->setSpaceValues(data.data(), data.size());
  } else if (xtypep == NC_UINT) {

    std::vector<uint32_t> data;
    data.resize(lenp);
    if ((retval = nc_get_var(grpid, varid, data.data()))) {
      return false;
    }
    pdim->setSpaceValues(data.data(), data.size());
  }
  return true;
#else
  std::cerr << __FILE__ << " TINC built without NetCDF support" << std::endl;
  return false;
#endif
}

#ifdef TINC_HAS_HDF5
al::VariantType nctypeToTincType(nc_type nctype) {
  // TODO complete support for all netcdf types
  switch (nctype) {
  case NC_STRING:
    return al::VariantType::VARIANT_STRING;
  case NC_FLOAT:
    return al::VariantType::VARIANT_FLOAT;
  case NC_DOUBLE:
    return al::VariantType::VARIANT_DOUBLE;
  case NC_BYTE:
    return al::VariantType::VARIANT_INT8;
  case NC_UBYTE:
    return al::VariantType::VARIANT_UINT8;
  case NC_SHORT:
    return al::VariantType::VARIANT_INT16;
  case NC_USHORT:
    return al::VariantType::VARIANT_UINT16;
  case NC_INT:
    return al::VariantType::VARIANT_INT32;
  case NC_UINT:
    return al::VariantType::VARIANT_UINT32;
  case NC_INT64:
    return al::VariantType::VARIANT_INT64;
  case NC_UINT64:
    return al::VariantType::VARIANT_UINT64;
  }
  return al::VariantType::VARIANT_FLOAT;
}
#endif

bool ParameterSpace::readDimensionsInNetCDFFile(
    std::string filename,
    std::vector<std::shared_ptr<ParameterSpaceDimension>> &newDimensions) {
#ifdef TINC_HAS_HDF5
  int ncid, retval;
  int num_state_grps;
  int state_grp_ids[16];
  int num_parameters;
  int parameters_ids[16];
  int num_conditions;
  int conditions_ids[16];
  nc_type xtypep;
  int ndimsp;
  int dimidsp[32];
  // FIXME need to check size of dimensions first
  int *nattsp = nullptr;
  size_t lenp;
  int varid;

  int internal_state_grpid;
  int parameters_grpid;
  int conditions_grpid;

  if ((retval = nc_open(filename.c_str(), NC_NOWRITE | NC_SHARE, &ncid))) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return false;
  }

  // Get main group ids
  if (nc_inq_grp_ncid(ncid, "internal_dimensions", &internal_state_grpid) ==
      0) {
    if (nc_inq_grps(internal_state_grpid, &num_state_grps, state_grp_ids)) {
      return false;
    }

    // Read internal states variable data
    for (int i = 0; i < num_state_grps; i++) {
      char groupName[32];
      // FIXME need to check size of name first
      if (nc_inq_grpname(state_grp_ids[i], groupName)) {
        return false;
      }
      int varid;
      if ((retval = nc_inq_varid(state_grp_ids[i], "values", &varid))) {
        return false;
      }
      nc_type nctypeid;
      if ((retval = nc_inq_vartype(state_grp_ids[i], varid, &nctypeid))) {
        return false;
      }

      ParameterSpaceDimension *pdim = getDimension(groupName, "");
      if (!pdim) {
        auto newDim = std::make_shared<ParameterSpaceDimension>(
            groupName, "", nctypeToTincType(nctypeid));
        newDimensions.push_back(newDim);
        pdim = newDim.get();
      }
      if (!readNetCDFValues(state_grp_ids[i], pdim)) {
        return false;
      }
      pdim->conformSpace();
      pdim->sort();
      pdim->mRepresentationType = ParameterSpaceDimension::VALUE;
      //    std::cout << "internal state " << i << ":" << groupName
      //              << " length: " << lenp << std::endl;
    }
  } else {
    std::cout << "No group 'internal_dimensions' in " << filename << std::endl;
  }

  if (nc_inq_grp_ncid(ncid, "mapped_dimensions", &parameters_grpid) == 0) {
    if (nc_inq_grps(parameters_grpid, &num_parameters, parameters_ids)) {
      return false;
    }

    // Process mapped parameters
    for (int i = 0; i < num_parameters; i++) {
      char parameterName[32];
      // FIXME need to check size of name first
      if (nc_inq_grpname(parameters_ids[i], parameterName)) {
        return false;
      }
      if ((retval = nc_inq_varid(parameters_ids[i], "values", &varid))) {
        return false;
      }

      if ((retval = nc_inq_var(parameters_ids[i], varid, nullptr, &xtypep,
                               &ndimsp, dimidsp, nattsp))) {
        return false;
      }
      if ((retval = nc_inq_dimlen(parameters_ids[i], dimidsp[0], &lenp))) {
        return false;
      }
      std::vector<float> data;
      data.resize(lenp);
      if ((retval = nc_get_var(parameters_ids[i], varid, data.data()))) {
        return false;
      }
      // Now get ids
      if ((retval = nc_inq_varid(parameters_ids[i], "ids", &varid))) {
        return false;
      }

      if ((retval = nc_inq_var(parameters_ids[i], varid, nullptr, &xtypep,
                               &ndimsp, dimidsp, nattsp))) {
        return false;
      }
      if ((retval = nc_inq_dimlen(parameters_ids[i], dimidsp[0], &lenp))) {
        return false;
      }

      // Here we get a vector of char * from the ids dimension
      // We don't need to allocate the char * as netcdf provides allocated
      // null terminated strings
      std::vector<char *> idData;
      idData.resize(lenp);
      if ((retval =
               nc_get_var_string(parameters_ids[i], varid, idData.data()))) {
        return false;
      }
      int varid;
      if ((retval = nc_inq_varid(parameters_ids[i], "values", &varid))) {
        return false;
      }
      nc_type nctypeid;
      if ((retval = nc_inq_vartype(parameters_ids[i], varid, &nctypeid))) {
        return false;
      }

      ParameterSpaceDimension *pdim = getDimension(parameterName, "");
      if (!pdim) {
        auto newDim = std::make_shared<ParameterSpaceDimension>(parameterName);
        newDimensions.push_back(newDim);
        pdim = newDim.get();
      }

      pdim->setSpaceValues(data.data(), data.size());
      std::vector<std::string> newIds;
      for (size_t i = 0; i < lenp; i++) {
        newIds.push_back(idData[i]);
      }
      pdim->setSpaceIds(newIds);
      pdim->conformSpace();
      pdim->sort();

      pdim->mRepresentationType = ParameterSpaceDimension::ID;

      //      std::cout << "mapped parameter " << i << ":" << parameterName
      //                << " length: " << lenp << std::endl;
    }
  } else {
    std::cerr << "Error finding group 'mapped_dimensions' in " << filename
              << std::endl;
  }

  if (nc_inq_grp_ncid(ncid, "index_dimensions", &conditions_grpid) == 0) {
    if (nc_inq_grps(conditions_grpid, &num_conditions, conditions_ids)) {
      return false;
    }
    // Read conditions
    for (int i = 0; i < num_conditions; i++) {
      char conditionName[32];
      // FIXME need to check size of name first
      if (nc_inq_grpname(conditions_ids[i], conditionName)) {
        return false;
      }

      ParameterSpaceDimension *pdim = getDimension(conditionName, "");

      if (!pdim) {
        auto newDim = std::make_shared<ParameterSpaceDimension>(conditionName);
        newDimensions.push_back(newDim);
        pdim = newDim.get();
      }

      if (!readNetCDFValues(conditions_ids[i], pdim)) {
        return false;
      }

      pdim->conformSpace();
      pdim->sort();
      pdim->mRepresentationType = ParameterSpaceDimension::INDEX;
    }
  } else {
    std::cerr << "Error finding group 'index_dimensions' in " << filename
              << std::endl;
  }

  if ((retval = nc_close(ncid))) {
    return false;
  }

  return true;
#else
  std::cerr << __FILE__ << " TINC built without NetCDF support" << std::endl;
  return false;
#endif
}
std::string ParameterSpace::getRootPath() { return mRootPath; }

void ParameterSpace::setRootPath(std::string rootPath) {
  if (mCacheManager && rootPath != mRootPath &&
      al::File::conformDirectory(rootPath) != mRootPath) {
    // TODO should we do it instead of warning?
    std::cout << __FILE__
              << "WARNING: Root path changed for parameter space. But cache "
                 "enabled. You must call enableCache() to adjust root path "
                 "for cache"
              << std::endl;
  }
  if (!al::File::isDirectory(rootPath)) {
    al::Dir::make(rootPath);
  }
  mRootPath = al::File::conformDirectory(rootPath);
}

std::string
ParameterSpace::resolveTemplate(std::string fileTemplate,
                                std::map<std::string, size_t> indices) {
  std::map<ParameterSpaceDimension *, size_t> indecesOverride;
  for (auto *dim : getDimensions()) {
    auto found = indices.find(dim->getName());
    if (found != indices.end()) {
      indecesOverride[dim] = found->second;
    }
  }
  return resolveTemplate(fileTemplate, getDimensions(), indecesOverride);
}

std::string ParameterSpace::resolveTemplate(
    std::string fileTemplate, std::vector<ParameterSpaceDimension *> dimensions,
    std::map<ParameterSpaceDimension *, size_t> indecesOverride) {
  std::string resolvedName;
  size_t currentPos = 0;
  size_t beginPos = fileTemplate.find("%%", currentPos);
  resolvedName += fileTemplate.substr(0, beginPos);
  while (beginPos != std::string::npos) {
    auto endPos = fileTemplate.find("%%", beginPos + 2);
    if (endPos != std::string::npos) {
      auto token = fileTemplate.substr(beginPos + 2, endPos - beginPos - 2);

      std::string representation;
      auto representationSeparation = token.find(":");
      if (representationSeparation != std::string::npos) {
        representation = token.substr(representationSeparation + 1);
        token = token.substr(0, representationSeparation);
      }
      bool replaced = false;

      std::vector<std::string> allTokens;
      auto multiSeparation = token.find(",");
      size_t currentIndex = 0;
      while (multiSeparation != std::string::npos) {
        representation = token.substr(representationSeparation + 1);
        allTokens.push_back(token.substr(currentIndex, multiSeparation));
        currentIndex += multiSeparation + 1;
        multiSeparation = token.find(",", currentIndex);
      }
      if ((currentIndex < representationSeparation ||
           representationSeparation != std::string::npos) &&
          currentIndex < endPos) {
        allTokens.push_back(
            token.substr(currentIndex, endPos - currentIndex - 2));
      }

      if (allTokens.size() > 1) {
        std::vector<ParameterSpaceDimension *> multiDimensions;
        for (auto token : allTokens) {
          for (auto *dim : dimensions) {
            if (dim->getName() == token) {
              multiDimensions.push_back(dim);
              break;
            }
          }
        }
        std::string commonId = getCommonId(multiDimensions, indecesOverride);
        if (commonId.size() > 0) {
          resolvedName += commonId;
          replaced = true;
        }
      } else {
        bool overriden = false;
        for (auto overrideEntry : indecesOverride) {
          if (overrideEntry.first->getName() == token) {
            // Use provided index instead of current values
            auto &index = overrideEntry.second;
            auto dim = overrideEntry.first;
            if (representation.size() == 0) {
              switch (dim->getSpaceRepresentationType()) {
              case ParameterSpaceDimension::ID:
                representation = "ID";
                break;
              case ParameterSpaceDimension::VALUE:
                representation = "VALUE";
                break;
              case ParameterSpaceDimension::INDEX:
                representation = "INDEX";
                break;
              }
            }
            if (representation == "ID") {
              resolvedName += dim->idAt(index);
            } else if (representation == "VALUE") {
              resolvedName += std::to_string(dim->at(index));
            } else if (representation == "INDEX") {
              resolvedName += std::to_string(index);
            } else {
              std::cerr << "Representation error: " << representation
                        << std::endl;
            }

            replaced = true;
            overriden = true;
          }
        }
        if (!overriden) {
          // Use current value
          for (auto dim : dimensions) {
            if (dim->getName() == token) {
              if (representation.size() == 0) {
                switch (dim->getSpaceRepresentationType()) {
                case ParameterSpaceDimension::ID:
                  representation = "ID";
                  break;
                case ParameterSpaceDimension::VALUE:
                  representation = "VALUE";
                  break;
                case ParameterSpaceDimension::INDEX:
                  representation = "INDEX";
                  break;
                }
              }
              if (representation == "ID") {
                resolvedName += dim->getCurrentId();
              } else if (representation == "VALUE") {
                resolvedName += std::to_string(dim->getCurrentValue());
              } else if (representation == "INDEX") {
                auto index = dim->getCurrentIndex();
                if (index == SIZE_MAX) {
                  index = 0;
                }
                resolvedName += std::to_string(index);
              } else {
                std::cerr << "Representation error: " << representation
                          << std::endl;
              }

              replaced = true;
              break;
            }
          }
        }
      }
      if (!replaced) {
        std::cerr << __FILE__ << " ERROR: Template token not matched:" << token
                  << std::endl;
      }
    }
    currentPos = endPos + 2;
    beginPos = fileTemplate.find("%%", endPos + 2);

    if (beginPos != std::string::npos) {
      resolvedName += fileTemplate.substr(currentPos, beginPos - currentPos);
    } else {
      resolvedName += fileTemplate.substr(currentPos);
    }
  }
  return resolvedName;
}

void ParameterSpace::enableCache(std::string cachePath) {
  al::PushDirectory dir(getRootPath());
  if (mCacheManager) {
    std::cout << "Warning cache already enabled. Overwriting previous settings"
              << std::endl;
  }
  cachePath = al::File::conformDirectory(cachePath);
  if (!al::File::isDirectory(cachePath)) {
    if (!al::Dir::make(cachePath)) {
      std::cerr << "ERROR creating cache directory: " << cachePath << std::endl;
    }
  }
  mCacheManager = std::make_shared<CacheManager>(
      DistributedPath{std::string("tinc_cache.json"), cachePath, mRootPath});
  mCacheManager->updateFromDisk();
}

void ParameterSpace::disableCache() { mCacheManager = nullptr; }

void ParameterSpace::print(std::ostream &stream) {
  stream << " ParameterSpace " << this->getId() << " @ " << this << std::endl;
  for (auto dim : getDimensions()) {
    dim->print(stream);
  }
  stream << " ------------------------  " << std::endl;
}

bool ParameterSpace::readFromNetCDF(std::string ncFile) {
#ifdef TINC_HAS_NETCDF
  std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
  std::string filename = al::File::conformPathToOS(mRootPath) + ncFile;
  if (!readDimensionsInNetCDFFile(filename, newDimensions)) {
    return false;
  }

  for (auto &newDim : newDimensions) {
    registerDimension(newDim);
  }

  auto dimNames = getDimensionNames();

  std::map<std::string, size_t> currentIndices;
  for (auto dimension : mDimensions) {
    if (dimension->size() > 0) {
      currentIndices[dimension->getName()] = 0;
    }
  }
  bool done = false;
  std::vector<std::string> innerDimensions;
  while (!done) {
    auto path = generateRelativeRunPath(currentIndices, this);

    std::stringstream ss(path);
    std::string item;
    std::string subPath;
    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
      if (item == ".") {
        continue;
      }
      subPath += item + AL_FILE_DELIMITER_STR;
      auto subpathFile =
          al::File::conformPathToOS(mRootPath) + subPath + ncFile;
      if (al::File::exists(subpathFile)) {
        mSpecialDirs[subPath] = ncFile;
        std::vector<std::shared_ptr<ParameterSpaceDimension>>
            newInnerDimensions;
        if (readDimensionsInNetCDFFile(subpathFile, newInnerDimensions)) {

          for (auto &newDim : newInnerDimensions) {
            //            if (std::find(innerDimensions.begin(),
            //            innerDimensions.end(),
            //                          newDim->getName()) ==
            //                          innerDimensions.end()) {
            //              innerDimensions.push_back(newDim->getName());
            //            }
            registerDimension(newDim);
          }
        }
      }
    }
    done = incrementIndices(currentIndices);
  }

  //  for (auto &dimName : innerDimensions) {
  //    if (!getDimension(dimName)) {
  //      registerDimension(std::make_shared<ParameterSpaceDimension>(dimName));
  //    }
  //  }

#else
  std::cerr << "TINC built without NetCDF support. "
               "ParameterSpaceDimension::loadFromNetCDF() does not work."
            << std::endl;
#endif

  return true;
}

bool writeNetCDFValues(int datagrpid,
                       std::shared_ptr<ParameterSpaceDimension> ps) {
#ifdef TINC_HAS_HDF5
  int retval;
  int shuffle = 1;
  int deflate = 9;
  int dimid;
  int varid;
  if ((retval = nc_def_dim(datagrpid, "values_dim", ps->size(), &dimid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  if (ps->getSpaceDataType() == al::VariantType::VARIANT_FLOAT) {
    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_FLOAT, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    std::vector<float> values = ps->getSpaceValues<float>();
    nc_put_var(datagrpid, varid, values.data());
  } else if (ps->getSpaceDataType() == al::VariantType::VARIANT_UINT8) {

    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_UBYTE, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    std::vector<uint8_t> valuesInt = ps->getSpaceValues<uint8_t>();
    nc_put_var(datagrpid, varid, valuesInt.data());

  } else if (ps->getSpaceDataType() == al::VariantType::VARIANT_INT32) {

    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_INT, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    std::vector<int32_t> valuesInt = ps->getSpaceValues<int32_t>();
    nc_put_var(datagrpid, varid, valuesInt.data());
  } else if (ps->getSpaceDataType() == al::VariantType::VARIANT_UINT32) {

    int dimidsp[1] = {dimid};
    if ((retval =
             nc_def_var(datagrpid, "values", NC_INT, 1, dimidsp, &varid))) {

      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }
    if ((retval = nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
      std::cerr << nc_strerror(retval) << std::endl;
      return false;
    }

    std::vector<uint32_t> valuesInt = ps->getSpaceValues<uint32_t>();
    nc_put_var(datagrpid, varid, valuesInt.data());
  }
  // FIXME add support for the rest of the data types.
  return true;
#else
  std::cerr << __FILE__ << " TINC built without NetCDF support" << std::endl;
  return false;
#endif
}

bool ParameterSpace::writeToNetCDF(std::string fileName) {
#ifdef TINC_HAS_HDF5
  int retval;
  int ncid;
  fileName = al::File::conformPathToOS(mRootPath) + fileName;

  if ((retval = nc_create(fileName.c_str(), NC_CLOBBER | NC_NETCDF4, &ncid))) {
    return false;
  }
  int grpid;
  if ((retval = nc_def_grp(ncid, "internal_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }

  for (auto psd : mDimensions) {
    if (psd->mRepresentationType == ParameterSpaceDimension::VALUE) {
      int datagrpid;
      if ((retval = nc_def_grp(grpid, psd->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, psd)) {
        return false;
      }
    }
  }

  if ((retval = nc_def_grp(ncid, "index_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  for (auto psd : mDimensions) {
    if (psd->mRepresentationType == ParameterSpaceDimension::INDEX) {
      int datagrpid;
      if ((retval = nc_def_grp(grpid, psd->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, psd)) {
        return false;
      }
    }
  }

  if ((retval = nc_def_grp(ncid, "mapped_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  for (auto psd : mDimensions) {
    if (psd->mRepresentationType == ParameterSpaceDimension::ID) {
      int shuffle = 1;
      int deflate = 9;
      int datagrpid;
      int varid;
      int dimid;
      if ((retval = nc_def_grp(grpid, psd->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, psd)) {
        return false;
      }

      // Insert ids --------
      if ((retval = nc_def_dim(datagrpid, "id_dim", psd->size(), &dimid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }

      int dimidsp[1] = {dimid};
      if ((retval =
               nc_def_var(datagrpid, "ids", NC_STRING, 1, dimidsp, &varid))) {

        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }

      if ((retval =
               nc_def_var_deflate(datagrpid, varid, shuffle, 1, deflate))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      auto ids = psd->getSpaceIds();
      char **idArray = (char **)calloc(ids.size(), sizeof(char *));
      size_t start[1] = {0};
      size_t count[1] = {ids.size()};
      for (size_t i = 0; i < ids.size(); i++) {
        idArray[i] = (char *)ids[i].c_str();
#ifdef __STDC_LIB_EXT1__
        strncpy_s(idArray[i], ids.size(), ids[i].data(), ids[i].size());
#elif defined(AL_WINDOWS)
        strncpy_s(idArray[i], ids.size(), ids[i].data(), ids[i].size());
#else
        strncpy(idArray[i], ids[i].data(), ids[i].size());
#endif
      }
      if ((retval = nc_put_vara_string(datagrpid, varid, start, count,
                                       (const char **)idArray))) {
        std::cerr << nc_strerror(retval) << std::endl;

        for (size_t i = 0; i < ids.size(); i++) {
          free(idArray[i]);
        }
        free(idArray);
        return false;
      }
      free(idArray);
    }
  }

  if ((retval = nc_close(ncid))) {
    return false;
  }
  //  std::map<std::string, size_t> indices;
  //  for (auto &ps : mappedParameters) {
  //    indices[ps->getName()] = 0;
  //  }
  //  for (auto &ps : conditionParameters) {
  //    indices[ps->getName()] = 0;
  //  }

  //  do {
  //    std::string newPath = generateRelativePath(indices);
  //    std::stringstream ss(newPath);
  //    std::string item;
  //    std::vector<std::string> newPathComponents;
  //    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
  //      newPathComponents.push_back(std::move(item));
  //    }

  //    auto newIt = newPathComponents.begin();

  //    std::string subPath;
  //    while (newIt != newPathComponents.end()) {
  //      subPath += *newIt + AL_FILE_DELIMITER_STR;
  //      std::cout << "Writing parameter space at " << subPath <<
  //      std::endl;
  //      newIt++;
  //    }
  //  } while (!incrementIndices(indices));
  return true;
#else
  std::cerr << __FILE__ << " TINC built without NetCDF support" << std::endl;
  return false;
#endif
}

void ParameterSpace::updateParameterSpace(ParameterSpaceDimension *ps) {
  if (mSpecialDirs.size() == 0) {
    return; // No need to check
  }

  if (isFilesystemDimension(ps->getName())) {
    std::map<std::string, size_t> indices;
    for (auto dimension : mDimensions) {
      if (isFilesystemDimension(dimension->getName())) {
        indices[dimension->getName()] = dimension->getCurrentIndex();
      }
    }
    std::string oldPath = generateRelativeRunPath(indices, this);
    std::stringstream ss(oldPath);
    std::string item;
    std::vector<std::string> oldPathComponents;
    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
      oldPathComponents.push_back(std::move(item));
    }

    auto newPath = getCurrentRelativeRunPath();
    std::stringstream ss2(newPath);
    std::vector<std::string> newPathComponents;
    while (std::getline(ss2, item, AL_FILE_DELIMITER)) {
      newPathComponents.push_back(std::move(item));
    }

    auto newIt = newPathComponents.begin();
    auto oldIt = oldPathComponents.begin();

    std::string oldSubPath;
    std::string subPath;
    bool needsRefresh = false;

    while (oldIt != oldPathComponents.end() &&
           newIt != newPathComponents.end()) {
      if (*oldIt != *newIt) {
        subPath += *newIt + AL_FILE_DELIMITER_STR;
        oldSubPath += *oldIt + AL_FILE_DELIMITER_STR;
        if (mSpecialDirs.find(subPath) != mSpecialDirs.end() ||
            mSpecialDirs.find(oldSubPath) != mSpecialDirs.end()) {
          needsRefresh = true;
          break;
        }
      }
      oldIt++;
      newIt++;
    }

    if (needsRefresh) {
      // For now, recreate whole paramter space, this could be optimized in
      // the future through caching
      std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
      std::string filename =
          al::File::conformPathToOS(mRootPath) + "parameter_space.nc";
      if (!readDimensionsInNetCDFFile(filename, newDimensions)) {
        std::cerr << "ERROR reading root parameter space" << std::endl;
      }

      for (auto newDim : newDimensions) {
        registerDimension(newDim);
      }
      newDimensions.clear();
      // FIXME remove dimensions in ParameterSpace that are no longer used

      subPath.clear();
      newIt = oldPathComponents.begin();
      while (newIt != oldPathComponents.end()) {
        subPath += *newIt + AL_FILE_DELIMITER_STR;
        if (mSpecialDirs.find(subPath) != mSpecialDirs.end() &&
            al::File::exists(al::File::conformPathToOS(mRootPath) + subPath +
                             mSpecialDirs[subPath])) {
          std::cout << "Loading parameter space at " << subPath << std::endl;
          readDimensionsInNetCDFFile(al::File::conformPathToOS(mRootPath) +
                                         subPath + mSpecialDirs[subPath],
                                     newDimensions);
        }
        newIt++;
      }

      for (auto newDim : newDimensions) {
        registerDimension(newDim);
      }
    }
  }
}

CacheEntry ParameterSpace::cacheEntryForProcessor(Processor &processor) {

  CacheEntry entry;
  entry.sourceInfo.type = al::demangle(typeid(processor).name());
  entry.sourceInfo.tincId = processor.getId();
  entry.sourceInfo.fileDependencies = {};     // FIXME
  entry.sourceInfo.commandLineArguments = ""; // FIXME

  for (auto dim : mDimensions) {
    SourceArgument arg;
    arg.id = dim->getName();
    auto *param = dim->getParameterMeta();
    if (al::Parameter *p = dynamic_cast<al::Parameter *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterBool *p =
                   dynamic_cast<al::ParameterBool *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterString *p =
                   dynamic_cast<al::ParameterString *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterInt *p = dynamic_cast<al::ParameterInt *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterInt8 *p =
                   dynamic_cast<al::ParameterInt8 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterInt16 *p =
                   dynamic_cast<al::ParameterInt16 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterInt64 *p =
                   dynamic_cast<al::ParameterInt64 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterUInt8 *p =
                   dynamic_cast<al::ParameterUInt8 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterUInt16 *p =
                   dynamic_cast<al::ParameterUInt16 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterUInt32 *p =
                   dynamic_cast<al::ParameterUInt32 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterUInt64 *p =
                   dynamic_cast<al::ParameterUInt64 *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterDouble *p =
                   dynamic_cast<al::ParameterDouble *>(param)) {
      arg.setValue(p->get());
    }
    // TODO implement support for all types
    /*else if (al::ParameterVec3 *p =
                 dynamic_cast<al::ParameterVec3 *>(param)) {
      mParameterValue = new al::ParameterVec3(*p);
    } else if (al::ParameterVec4 *p =
                 dynamic_cast<al::ParameterVec4 *>(param)) {
      mParameterValue = new al::ParameterVec4(*p);
    } else if (al::ParameterColor *p =
                 dynamic_cast<al::ParameterColor *>(param)) {
      mParameterValue = new al::ParameterColor(*p);
    } else if (al::ParameterPose *p =
                 dynamic_cast<al::ParameterPose *>(param)) {
      mParameterValue = new al::ParameterPose(*p);
    } */
    else if (al::ParameterMenu *p = dynamic_cast<al::ParameterMenu *>(param)) {
      arg.setValue(p->get());
    } else if (al::ParameterChoice *p =
                   dynamic_cast<al::ParameterChoice *>(param)) {
      arg.setValue(p->get());
    } else if (al::Trigger *p = dynamic_cast<al::Trigger *>(param)) {
      arg.setValue(p->get());
    } else {
      std::cerr << __FUNCTION__ << ": Unsupported Parameter Type" << std::endl;
      arg.setValue(al::VariantValue());
    }
    entry.sourceInfo.arguments.push_back(std::move(arg));
  }

  for (auto *param : processor.getDependencies()) {
    SourceArgument arg;
    arg.id = param->getName();
    std::vector<al::VariantValue> fields;
    param->getParameterMeta()->getFields(fields);
    if (fields.size() == 1) {
      switch (fields[0].type()) {
      case al::VariantType::VARIANT_FLOAT:
        arg.setValue(fields[0].get<float>());
        break;
      case al::VariantType::VARIANT_DOUBLE:
        arg.setValue(fields[0].get<double>());
        break;
      case al::VariantType::VARIANT_STRING:
        arg.setValue(fields[0].get<std::string>());
        break;
      case al::VariantType::VARIANT_INT64:
        arg.setValue(fields[0].get<int64_t>());
        break;
      case al::VariantType::VARIANT_INT32:
        arg.setValue(fields[0].get<int32_t>());
        break;
      case al::VariantType::VARIANT_INT16:
        arg.setValue(fields[0].get<int16_t>());
        break;
      case al::VariantType::VARIANT_INT8:
        arg.setValue(fields[0].get<int8_t>());
        break;
      case al::VariantType::VARIANT_UINT64:
        arg.setValue(fields[0].get<uint64_t>());
        break;
      case al::VariantType::VARIANT_UINT32:
        arg.setValue(fields[0].get<uint32_t>());
        break;
      case al::VariantType::VARIANT_UINT16:
        arg.setValue(fields[0].get<uint16_t>());
        break;
      case al::VariantType::VARIANT_UINT8:
        arg.setValue(fields[0].get<uint8_t>());
        break;
      case al::VariantType::VARIANT_NONE:
      case al::VariantType::VARIANT_BOOL:
      case al::VariantType::VARIANT_CHAR:
      case al::VariantType::VARIANT_VARIANT_VECTOR:
      case al::VariantType::VARIANT_VECTOR_OFFSET:

        std::cerr << __FILE__ << ":" << __LINE__
                  << " Unsupported parameter: " << param->getName()
                  << std::endl;
        break;
      }
    } else {
      std::cerr << "Parameters with multiple values not supported" << std::endl;
    }
    entry.sourceInfo.dependencies.push_back(std::move(arg));
  }

  return entry;
}

bool ParameterSpace::executeProcess(Processor &processor, bool recompute) {
  std::time_t startTime =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  CacheEntry entry;

  bool cacheRestored = false;
  // TODO this is overriding args passed
  if (mCacheManager) {
    // Create sourceInfo section for cache entr
    entry = cacheEntryForProcessor(processor);
    auto cacheFiles = mCacheManager->findCache(entry.sourceInfo, false);

    // TODO caching is currently done by copying. There should also be an option
    // for in-place caching.
    if (cacheFiles.size() > 0) {
      auto outputFiles = processor.getOutputFileNames();
      if (outputFiles.size() != cacheFiles.size()) {
        recompute = true;
        std::cout << "Warning processor output files and cache files mismatch"
                  << std::endl;
      } else {
        cacheRestored = true;
        for (size_t i = 0; i < cacheFiles.size(); i++) {
          if (!al::File::copy(
                  mCacheManager->cacheDirectory() + cacheFiles.at(i),
                  processor.getOutputDirectory() + outputFiles.at(i))) {
            std::cerr << "ERROR restoring cache from"
                      << mCacheManager->cacheDirectory() + cacheFiles.at(i)
                      << " to "
                      << processor.getOutputDirectory() + outputFiles.at(i)
                      << std::endl;
            cacheRestored = false;
            recompute = true;
            break;
          } else {
            std::cout << "Cache restored from: "
                      << mCacheManager->cacheDirectory() + cacheFiles.at(i)
                      << std::endl;
          }
        }
      }
    } else {
      recompute = true;
    }
  } else {
    // Always recompute if not caching
    recompute = true;
  }
  bool ret = true;
  if (!cacheRestored) {
    ret = processor.process(recompute);
  }

  if (mCacheManager) {
    std::vector<FileDependency> cacheFiles;
    if (!cacheRestored) {
      for (auto filename : processor.getOutputFileNames()) {

#ifdef TINC_CPP_17
        uint64_t size = std::filesystem::file_size(
            processor.getOutputDirectory() + filename);
#else
        uint64_t size =
            al::File::sizeFile(processor.getOutputDirectory() + filename);
#endif
        uint32_t crc = CacheManager::computeCrc32(
            processor.getOutputDirectory() + filename);
        std::string hash = std::to_string(crc);
        std::string parameterPrefix;
        for (auto *dep : processor.getDependencies()) {

          std::vector<al::VariantValue> fields;
          dep->getParameterMeta()->getFields(fields);
          if (fields.size() == 1) {
            parameterPrefix += fields[0].toString() + "_";
          } else {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " ERROR: parameters with multiple values not supported"
                      << std::endl;
          }
        }
        for (auto dim : mDimensions) {
          parameterPrefix += "%%" + dim->getName() + "%%_";
        }
        parameterPrefix += "_" + hash + "_";
        parameterPrefix =
            ProcessorScript::sanitizeName(resolveTemplate(parameterPrefix));
        // FIXME compress filename to ensure path is not too long. Perhaps use a
        // hash for the path when too long?
        std::string cacheFilename =
            mCacheManager->cacheDirectory() + "/" + parameterPrefix + filename;
        if (al::File::exists(cacheFilename)) {
          // FIXME handle case when file exists.
        }
        if (!al::File::copy(processor.getOutputDirectory() + filename,
                            cacheFilename)) {
          std::cerr << "ERROR creating cache file " << cacheFilename
                    << " Cache entry not created. " << std::endl;
          return ret;
        }

#ifdef TINC_CPP_17
        auto modifiedTime = std::filesystem::last_write_time(cacheFilename);
        std::time_t cftime = __tinc_to_time_t(modifiedTime);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&cftime), "%FT%T%z");
#else
        al::File f(cacheFilename);
        struct stat s;
        std::stringstream ss;
        if (::stat(cacheFilename.c_str(), &s) == 0) {
          char time[64];
          std::strftime(time, 64, "%FT%T%z", std::localtime(&s.st_mtime));
          ss << time;
        }
#endif

        cacheFiles.push_back(
            FileDependency{DistributedPath{parameterPrefix + filename, ""},
                           ss.str(), size, hash});
      }

      for (auto filename : processor.getInputFileNames()) {

        std::string filePath = processor.getInputDirectory() + filename;

        FileDependency dep;
#ifdef TINC_CPP_17
        if (std::filesystem::exists(filePath)) {
          dep.file = DistributedPath(
              filename, processor.getInputDirectory()); // TODO enrich meta data
          auto modifiedTime = std::filesystem::last_write_time(filePath);
          std::time_t cftime = __tinc_to_time_t(modifiedTime);

          std::stringstream ss;
          ss << std::put_time(std::localtime(&cftime), "%FT%T%z");
          dep.size = std::filesystem::file_size(filePath);
#else
        if (al::File::exists(filePath)) {
          dep.file = DistributedPath(
              filename, processor.getInputDirectory()); // TODO enrich meta data
          al::File f(filePath);
          struct stat s;
          std::stringstream ss;
          if (::stat(filePath.c_str(), &s) == 0) {
            char time[64];
            std::strftime(time, 64, "%FT%T%z", std::localtime(&s.st_mtime));
            ss << time;
          }
          dep.size = al::File::sizeFile(filePath);
#endif

          dep.modified = ss.str();
          uint32_t crc = CacheManager::computeCrc32(filename);
          dep.hash = std::to_string(crc);
          entry.sourceInfo.fileDependencies.push_back(dep);
        } else {
          std::cerr << __FILE__ << ":" << __LINE__
                    << " ERROR input file to processor nto found. Not adding "
                       "dependency to cache."
                    << std::endl;
        }
      }

      entry.sourceInfo.workingPath = DistributedPath(); // FIXME

      std::stringstream ss;
      ss << std::put_time(std::localtime(&startTime), "%FT%T%z");
      entry.timestampStart = ss.str();
      // Leave end timestamp for last
      //    entry.cacheHits = 23;
      entry.files = cacheFiles;
      entry.stale = false;

      entry.userInfo.userName = "User";    // FIXME
      entry.userInfo.userHash = "UserHas"; // FIXME
      entry.userInfo.ip = "localhost";     // FIXME
      entry.userInfo.port = 12345;         // FIXME
      entry.userInfo.server = true;        // FIXME

      std::time_t endTime = std::chrono::system_clock::to_time_t(
          std::chrono::system_clock::now());

      ss.clear();
      ss.seekp(0);
      ss << std::put_time(std::localtime(&endTime), "%FT%T%z");
      entry.timestampEnd = ss.str();

      mCacheManager->appendEntry(entry);
      mCacheManager->writeToDisk();
    }
  }
  return ret;
}
