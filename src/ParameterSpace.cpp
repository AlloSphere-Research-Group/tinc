#include "tinc/ParameterSpace.hpp"

#include "al/io/al_File.hpp"

#ifdef TINC_HAS_HDF5
#include <netcdf.h>
#endif

#if defined(AL_OSX) || defined(AL_LINUX) || defined(AL_EMSCRIPTEN)
#include <unistd.h>
#else

#endif

#include <iostream>

using namespace tinc;

ParameterSpace::~ParameterSpace() { stopSweep(); }

std::shared_ptr<ParameterSpaceDimension>
ParameterSpace::getDimension(std::string name, std::string group) {

  std::unique_lock<std::mutex> lk(mSpaceLock);
  if (parameterNameMap.find(name) != parameterNameMap.end()) {
    name = parameterNameMap[name];
  }
  for (auto ps : getDimensions()) {
    if (ps->getName() == name && ps->getGroup() == group) {
      return ps;
    } else if (group == "" && ps->getFullAddress() == name) {
      return ps;
    }
  }
  return nullptr;
}

std::shared_ptr<ParameterSpaceDimension>
ParameterSpace::newDimension(std::string name, std::string group,
                             ParameterSpaceDimension::RepresentationType type,
                             al::DiscreteParameterValues::Datatype datatype) {
  auto newDim =
      std::make_shared<ParameterSpaceDimension>(name, group, datatype);

  newDim->mRepresentationType = type;
  registerDimension(newDim);
  return newDim;
}

void ParameterSpace::registerDimension(
    std::shared_ptr<ParameterSpaceDimension> dimension, al::Socket *src) {
  std::unique_lock<std::mutex> lk(mSpaceLock);
  for (auto dim : getDimensions()) {
    if (dim->getName() == dimension->getName() &&
        dim->getGroup() == dimension->getGroup()) {
      // FIXME check data type
      if (dim->mSpaceValues.getDataType() ==
          dimension->mSpaceValues.getDataType()) {
        dim->mSpaceValues.append(dimension->mSpaceValues.getValuesPtr(),
                                 dimension->mSpaceValues.size());
        dim->mSpaceValues.setIds(dimension->mSpaceValues.getIds());
        dim->mRepresentationType = dimension->getSpaceRepresentationType();

        //      std::cout << "Updated dimension: " << dimension->getName() <<
        //      std::endl;
        onDimensionRegister(dim.get(), this, src);
        return;
      } else {
        std::cout << "WARNING: Dimension datatype change." << std::endl;
      }
    }
  }

  if (al::ParameterBool *p =
          dynamic_cast<al::ParameterBool *>(dimension->parameterMeta())) {
    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](float value) {
      //    std::cout << value << dimension->getName() << std::endl;
      float oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });
    mDimensions.push_back(dimension);
    onDimensionRegister(dimension.get(), this, src);
  } else if (al::Parameter *p =
                 dynamic_cast<al::Parameter *>(dimension->parameterMeta())) {
    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](float value) {
      //    std::cout << value << dimension->getName() << std::endl;
      float oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });
    mDimensions.push_back(dimension);
    onDimensionRegister(dimension.get(), this, src);
  } else if (al::ParameterInt *p =
                 dynamic_cast<al::ParameterInt *>(dimension->parameterMeta())) {
    auto &param = *p;
    param.registerChangeCallback([dimension, &param, this](int32_t value) {
      //    std::cout << value << dimension->getName() << std::endl;
      int32_t oldValue = param.get();
      param.setNoCalls(value);

      this->updateParameterSpace(dimension.get());
      this->onValueChange(dimension.get(), this);
      param.setNoCalls(oldValue);
      // The internal parameter will get set internally to the new value
      // later on inside the Parameter classes
    });
    mDimensions.push_back(dimension);
    onDimensionRegister(dimension.get(), this, src);
  } else {
    // FIXME implement for all parameter types
    std::cerr << "Support for parameter type not implemented in dimension "
              << __FILE__ << ":" << __LINE__ << std::endl;
  }
}

void ParameterSpace::removeDimension(std::string name, std::string group,
                                     al::Socket *src) {
  auto it = mDimensions.begin();
  while (it < mDimensions.end()) {
    if ((*it)->getName() == name && (*it)->getGroup() == group) {
      onDimensionRemove(it->get(), this, src);
      it = mDimensions.erase(it);
      break;
    } else if (group == "" && (*it)->getFullAddress() == name) {
      onDimensionRemove(it->get(), this, src);
      it = mDimensions.erase(it);
      break;
    } else {
      ++it;
    }
  }

  std::cout << "after while" << std::endl;
  if (it < mDimensions.end())
    std::cout << "it hasn't reached end" << std::endl;
}

std::vector<std::shared_ptr<ParameterSpaceDimension>>
ParameterSpace::getDimensions() {
  return mDimensions;
}

std::vector<std::string> ParameterSpace::runningPaths() {
  std::vector<std::string> paths;

  std::map<std::string, size_t> currentIndeces;
  for (auto dimension : mDimensions) {
    currentIndeces[dimension->getName()] = 0;
  }
  bool done = false;
  while (!done) {
    done = true;
    auto path = al::File::conformPathToOS(rootPath) + currentRunPath();
    // TODO write more efficient way to determine if a dimension affects the
    // filesystem. Perhaps analyze before running this loop to prune dimensions
    // that don't affect the filesystem
    if (path.size() > 0 &&
        std::find(paths.begin(), paths.end(), path) == paths.end()) {
      paths.push_back(path);
    }
    done = incrementIndeces(currentIndeces);
  }
  return paths;
}

std::string ParameterSpace::currentRunPath() {
  std::map<std::string, size_t> indeces;
  {
    std::unique_lock<std::mutex> lk(mSpaceLock);
    for (auto ps : mDimensions) {
      //      if (ps->isFilesystemDimension()) {
      indeces[ps->getName()] = ps->getCurrentIndex();
      //      }
    }
  }
  return generateRelativeRunPath(indeces, this);
}

std::vector<std::string> ParameterSpace::dimensionNames() {
  std::unique_lock<std::mutex> lk(mSpaceLock);
  std::vector<std::string> dimensionNames;
  for (auto dim : mDimensions) {
    dimensionNames.push_back(dim->getName());
  }
  return dimensionNames;
}

bool ParameterSpace::isFilesystemDimension(std::string dimensionName) {
  auto dim = getDimension(dimensionName);
  if (dim) {
    // This should be enough of a check, or should we check all possible
    // values?
    std::map<std::string, size_t> indeces;
    indeces[dim->getName()] = {0};
    auto path0 = generateRelativeRunPath(indeces, this);
    indeces[dim->getName()] = {1};
    auto path1 = generateRelativeRunPath(indeces, this);
    if (path0 != path1) {
      return true;
    }
  }
  return false;
}

void ParameterSpace::clear() {
  std::unique_lock<std::mutex> lk(mSpaceLock);
  mDimensions.clear();
  mSpecialDirs.clear();
}

bool ParameterSpace::incrementIndeces(
    std::map<std::string, size_t> &currentIndeces) {

  for (auto &dimensionIndex : currentIndeces) {
    auto dimension = getDimension(dimensionIndex.first);
    dimensionIndex.second++;
    if (dimensionIndex.second >= dimension->size()) {
      dimensionIndex.second = 0;
    } else {
      return false;
    }
  }
  return true;
}

void ParameterSpace::sweep(Processor &processor,
                           std::vector<std::string> dimensionNames_,
                           bool recompute) {
  uint64_t sweepCount = 0;
  uint64_t sweepTotal = 1;
  mSweepRunning = true;
  if (dimensionNames_.size() == 0) {
    dimensionNames_ = dimensionNames();
  }
  for (auto dimensionName : dimensionNames_) {
    auto dim = getDimension(dimensionName);
    if (dim) {
      sweepTotal *= dim->size();
    } else {
      std::cerr << __FUNCTION__
                << " ERROR: dimension not found: " << dimensionName
                << std::endl;
    }
  }

  std::map<std::string, size_t> previousIndeces;
  for (auto dimName : dimensionNames_) {
    auto dim = getDimension(dimName);
    if (dim) {
      previousIndeces[dimName] = dim->getCurrentIndex();
      dim->setCurrentIndex(0);
    }
  }

  while (mSweepRunning) {
    {
      std::unique_lock<std::mutex> lk(mSpaceLock);
      for (auto dim : mDimensions) {
        if (dim->mRepresentationType == ParameterSpaceDimension::VALUE) {
          processor.configuration[dim->getName()] = dim->getCurrentValue();
        } else if (dim->mRepresentationType == ParameterSpaceDimension::ID) {
          processor.configuration[dim->getName()] = dim->getCurrentId();
        } else if (dim->mRepresentationType == ParameterSpaceDimension::INDEX) {
          assert(dim->getCurrentIndex() < std::numeric_limits<int64_t>::max());
          processor.configuration[dim->getName()] =
              (int64_t)dim->getCurrentIndex();
        }
      }
    }

    auto path = currentRunPath();
    if (path.size() > 0) {
      // TODO allow fine grained options of what directory to set
      processor.setRunningDirectory(path);
    }
    sweepCount++;
    if (!processor.process(recompute) && !processor.ignoreFail) {
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
      if (dim->getCurrentIndex() == dim->size() - 1) {
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
  for (auto previousIndex : previousIndeces) {
    getDimension(previousIndex.first)->setCurrentIndex(previousIndex.second);
  }
  mSweepRunning = false;
}

void ParameterSpace::sweepAsync(Processor &processor,
                                std::vector<std::string> dimensions,
                                bool recompute) {
  if (mAsyncProcessingThread || mAsyncPSCopy) {
    stopSweep();
  }
  mAsyncPSCopy = std::make_shared<ParameterSpace>();
  {
    std::unique_lock<std::mutex> lk(mSpaceLock);
    for (auto dim : ParameterSpace::mDimensions) {
      auto dimCopy = dim->deepCopy();
      mAsyncPSCopy->registerDimension(dimCopy);
    }
    mAsyncPSCopy->onSweepProcess = onSweepProcess;
    mAsyncPSCopy->onValueChange = onValueChange;
    mAsyncPSCopy->generateRelativeRunPath = generateRelativeRunPath;
    mAsyncPSCopy->mCurrentPathTemplate = mCurrentPathTemplate;
  }
  mAsyncProcessingThread = std::make_unique<std::thread>([=, &processor]() {
    mAsyncPSCopy->sweep(processor, dimensions, recompute);
  });
}

bool ParameterSpace::createDataDirectories() {
  for (auto path : runningPaths()) {
    if (!al::File::isDirectory(path)) {
      if (!al::Dir::make(path)) {
        return false;
      }
    }
  }
  return true;
}

bool ParameterSpace::cleanDataDirectories() {
  for (auto path : runningPaths()) {
    if (al::File::isDirectory(path)) {
      if (!al::Dir::removeRecursively(path)) {
        return false;
      }
    }
  }
  return createDataDirectories();
}

void ParameterSpace::stopSweep() {
  mSweepRunning = false;
  if (mAsyncPSCopy) {
    mAsyncPSCopy->stopSweep();
  }
  if (mAsyncProcessingThread) {
    mAsyncProcessingThread->join();
    mAsyncProcessingThread = nullptr;
  }
  mAsyncPSCopy = nullptr;
}

bool readNetCDFValues(int grpid,
                      std::shared_ptr<ParameterSpaceDimension> pdim) {
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
}

#ifdef TINC_HAS_HDF5
ParameterSpaceDimension::Datatype nctypeToTincType(nc_type nctype) {
  // TODO complete support for all netcdf types
  switch (nctype) {
  case NC_STRING:
    return ParameterSpaceDimension::Datatype::STRING;
  case NC_FLOAT:
    return ParameterSpaceDimension::Datatype::FLOAT;
  case NC_DOUBLE:
    return ParameterSpaceDimension::Datatype::DOUBLE;
  case NC_BYTE:
    return ParameterSpaceDimension::Datatype::INT8;
  case NC_UBYTE:
    return ParameterSpaceDimension::Datatype::UINT8;
  case NC_INT:
    return ParameterSpaceDimension::Datatype::INT32;
  case NC_UINT:
    return ParameterSpaceDimension::Datatype::UINT32;
  case NC_INT64:
    return ParameterSpaceDimension::Datatype::INT64;
  case NC_UINT64:
    return ParameterSpaceDimension::Datatype::UINT64;
  }
  return ParameterSpaceDimension::Datatype::FLOAT;
}
#endif

bool ParameterSpace::readDimensionsInNetCDFFile(
    std::string filename,
    std::vector<std::shared_ptr<ParameterSpaceDimension>> &newDimensions) {
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
      std::shared_ptr<ParameterSpaceDimension> pdim;
      for (auto dim : getDimensions()) {
        if (dim->getName() == groupName && dim->getGroup().size() == 0) {
          pdim = dim;
          break;
        }
      }
      if (!pdim) {
        int varid;
        if ((retval = nc_inq_varid(state_grp_ids[i], "values", &varid))) {
          return false;
        }
        nc_type nctypeid;
        if ((retval = nc_inq_vartype(state_grp_ids[i], varid, &nctypeid))) {
          return false;
        }

        pdim = std::make_shared<ParameterSpaceDimension>(
            groupName, "", nctypeToTincType(nctypeid));
        newDimensions.push_back(pdim);
      }
      if (!readNetCDFValues(state_grp_ids[i], pdim)) {
        return false;
      }
      pdim->conformSpace();
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

      // FIXME this looks wrong, what should it do?
      std::vector<char *> idData;
      idData.resize(lenp * 80);
      if ((retval =
               nc_get_var_string(parameters_ids[i], varid, idData.data()))) {
        return false;
      }
      std::shared_ptr<ParameterSpaceDimension> pdim;
      for (auto dim : getDimensions()) {
        if (dim->getName() == parameterName && dim->getGroup().size() == 0) {
          pdim = dim;
          break;
        }
      }
      if (!pdim) {
        int varid;
        if ((retval = nc_inq_varid(state_grp_ids[i], "values", &varid))) {
          return false;
        }
        nc_type nctypeid;
        if ((retval = nc_inq_vartype(state_grp_ids[i], varid, &nctypeid))) {
          return false;
        }
        pdim = std::make_shared<ParameterSpaceDimension>(parameterName);
        newDimensions.push_back(pdim);
      }

      pdim->setSpaceValues(data.data(), data.size());
      std::vector<std::string> newIds;
      for (size_t i = 0; i < lenp; i++) {
        newIds.push_back(idData[i]);
      }
      pdim->setSpaceIds(newIds);

      pdim->conformSpace();
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
      std::shared_ptr<ParameterSpaceDimension> pdim;
      for (auto dim : getDimensions()) {
        if (dim->getName() == conditionName && dim->getGroup().size() == 0) {
          pdim = dim;
          break;
        }
      }
      if (!pdim) {
        pdim = std::make_shared<ParameterSpaceDimension>(conditionName);
        newDimensions.push_back(pdim);
      }

      if (!readNetCDFValues(conditions_ids[i], pdim)) {
        return false;
      }

      pdim->conformSpace();
      pdim->mRepresentationType = ParameterSpaceDimension::INDEX;
    }
  } else {
    std::cerr << "Error finding group 'index_dimensions' in " << filename
              << std::endl;
  }

  //  mParameterSpaces["time"]->append(timeSteps.data(), timeSteps.size());
  if ((retval = nc_close(ncid))) {
    return false;
  }

  return true;
}

std::string ParameterSpace::resolveFilename(std::string fileTemplate) {
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
      for (auto dim : getDimensions()) {
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

bool ParameterSpace::readFromNetCDF(std::string ncFile) {
#ifdef TINC_HAS_NETCDF
  std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
  std::string filename = al::File::conformPathToOS(rootPath) + ncFile;
  if (!readDimensionsInNetCDFFile(filename, newDimensions)) {
    return false;
  }

  for (auto newDim : newDimensions) {
    registerDimension(newDim);
  }

  auto dimNames = dimensionNames();

  std::map<std::string, size_t> currentIndeces;
  for (auto dimension : dimNames) {
    currentIndeces[dimension] = 0;
  }
  bool done = false;
  std::vector<std::string> innerDimensions;
  while (!done) {
    auto path = generateRelativeRunPath(currentIndeces, this);

    std::stringstream ss(path);
    std::string item;
    std::string subPath;
    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
      subPath += item + AL_FILE_DELIMITER_STR;
      if (al::File::exists(al::File::conformPathToOS(rootPath) + subPath +
                           ncFile)) {
        mSpecialDirs[subPath] = ncFile;
        std::vector<std::shared_ptr<ParameterSpaceDimension>>
            newInnerDimensions;
        if (readDimensionsInNetCDFFile(filename, newInnerDimensions)) {

          for (auto newDim : newInnerDimensions) {
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
    done = incrementIndeces(currentIndeces);
  }
  //  for (auto dimName : innerDimensions) {
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
  int retval;
  int shuffle = 1;
  int deflate = 9;
  int dimid;
  int varid;
  if ((retval = nc_def_dim(datagrpid, "values_dim", ps->size(), &dimid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  if (ps->getSpaceDataType() == al::DiscreteParameterValues::FLOAT) {
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
  } else if (ps->getSpaceDataType() == al::DiscreteParameterValues::UINT8) {

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

  } else if (ps->getSpaceDataType() == al::DiscreteParameterValues::INT32) {

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
  } else if (ps->getSpaceDataType() == al::DiscreteParameterValues::UINT32) {

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
}

bool ParameterSpace::writeToNetCDF(std::string fileName) {
  int retval;
  int ncid;
  fileName = al::File::conformPathToOS(rootPath) + fileName;

  if ((retval = nc_create(fileName.c_str(), NC_CLOBBER | NC_NETCDF4, &ncid))) {
    return false;
  }
  int grpid;
  if ((retval = nc_def_grp(ncid, "internal_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }

  for (auto ps : mDimensions) {
    if (ps->mRepresentationType == ParameterSpaceDimension::VALUE) {
      int datagrpid;
      if ((retval = nc_def_grp(grpid, ps->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, ps)) {
        return false;
      }
    }
  }

  if ((retval = nc_def_grp(ncid, "index_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  for (auto ps : mDimensions) {
    if (ps->mRepresentationType == ParameterSpaceDimension::INDEX) {
      int datagrpid;
      if ((retval = nc_def_grp(grpid, ps->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, ps)) {
        return false;
      }
    }
  }

  if ((retval = nc_def_grp(ncid, "mapped_dimensions", &grpid))) {
    std::cerr << nc_strerror(retval) << std::endl;
    return false;
  }
  for (auto ps : mDimensions) {
    if (ps->mRepresentationType == ParameterSpaceDimension::ID) {
      int shuffle = 1;
      int deflate = 9;
      int datagrpid;
      int varid;
      int dimid;
      if ((retval = nc_def_grp(grpid, ps->getName().c_str(), &datagrpid))) {
        std::cerr << nc_strerror(retval) << std::endl;
        return false;
      }
      if (!writeNetCDFValues(datagrpid, ps)) {
        return false;
      }

      // Insert ids --------
      if ((retval = nc_def_dim(datagrpid, "id_dim", ps->size(), &dimid))) {
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
      auto ids = ps->getSpaceIds();
      char **idArray = (char **)calloc(ids.size(), sizeof(char *));
      size_t start[1] = {0};
      size_t count[1] = {ids.size()};
      for (size_t i = 0; i < ids.size(); i++) {
        idArray[i] = (char *)calloc(ids[i].size(), sizeof(char));
        strncpy(idArray[i], ids[i].data(), ids[i].size());
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

      for (size_t i = 0; i < ids.size(); i++) {
        free(idArray[i]);
      }
      free(idArray);
    }
  }

  if ((retval = nc_close(ncid))) {
    return false;
  }
  //  std::map<std::string, size_t> indeces;
  //  for (auto ps : mappedParameters) {
  //    indeces[ps->getName()] = 0;
  //  }
  //  for (auto ps : conditionParameters) {
  //    indeces[ps->getName()] = 0;
  //  }

  //  do {
  //    std::string newPath = generateRelativePath(indeces);
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
  //  } while (!incrementIndeces(indeces));
  return true;
}

void ParameterSpace::updateParameterSpace(ParameterSpaceDimension *ps) {
  if (mSpecialDirs.size() == 0) {
    return; // No need to check
  }

  if (isFilesystemDimension(ps->getName())) {
    std::map<std::string, size_t> indeces;
    for (auto dimension : mDimensions) {
      if (isFilesystemDimension(dimension->getName())) {
        indeces[dimension->getName()] = dimension->getCurrentIndex();
      }
    }
    std::string oldPath = generateRelativeRunPath(indeces, this);
    std::stringstream ss(oldPath);
    std::string item;
    std::vector<std::string> oldPathComponents;
    while (std::getline(ss, item, AL_FILE_DELIMITER)) {
      oldPathComponents.push_back(std::move(item));
    }

    auto newPath = currentRunPath();
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
      // the
      // future through caching
      std::vector<std::shared_ptr<ParameterSpaceDimension>> newDimensions;
      std::string filename =
          al::File::conformPathToOS(rootPath) + "parameter_space.nc";
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
            al::File::exists(al::File::conformPathToOS(rootPath) + subPath +
                             mSpecialDirs[subPath])) {
          std::cout << "Loading parameter space at " << subPath << std::endl;
          readDimensionsInNetCDFFile(al::File::conformPathToOS(rootPath) +
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
