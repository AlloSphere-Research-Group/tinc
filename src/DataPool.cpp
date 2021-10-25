#include "tinc/DataPool.hpp"
#include "tinc/ProcessorScript.hpp"

#include "al/io/al_File.hpp"

#ifdef TINC_HAS_NETCDF
#include <netcdf.h>
#endif

#include <fstream>

using namespace tinc;

DataPool::DataPool(ParameterSpace &ps, std::string sliceCacheDir)
    : mParameterSpace(&ps) {
  //  if (sliceCacheDir.size() == 0) {
  //    sliceCacheDir = "";
  //  }
  auto cacheDirectory = al::File::conformDirectory(sliceCacheDir);
  if (!al::File::exists(cacheDirectory)) {
    if (!al::Dir::make(cacheDirectory)) {
      std::cerr << "ERROR creating directory: " << cacheDirectory << std::endl;
    }
  }
  mSliceCacheDirectory = cacheDirectory;
}

DataPool::DataPool(std::string id, ParameterSpace &ps,
                   std::string sliceCacheDir)
    : mParameterSpace(&ps) {
  mId = id;
  //  if (sliceCacheDir.size() == 0) {
  //    sliceCacheDir = al::File::currentPath();
  //  }
  auto cacheDirectory = al::File::conformDirectory(sliceCacheDir);
  if (!al::File::exists(cacheDirectory)) {
    if (!al::Dir::make(cacheDirectory)) {
      std::cerr << "ERROR creating directory: " << cacheDirectory << std::endl;
    }
  }
  mSliceCacheDirectory = cacheDirectory;
}

void DataPool::registerDataFile(std::string filename,
                                std::string dimensionInFile) {
  if (mDataFilenames.find(filename) != mDataFilenames.end()) {
    std::cout << "DataPool: Overwriting dimension in file " << filename
              << std::endl;
  }
  mDataFilenames[filename] = dimensionInFile;
  // TODO send to clients
}

std::string DataPool::createDataSlice(std::string field,
                                      std::string sliceDimension) {
  return createDataSlice(field, std::vector<std::string>{sliceDimension});
}

std::string
DataPool::createDataSlice(std::string field,
                          std::vector<std::string> sliceDimensions) {

  std::vector<std::string> filesystemDims;
  std::vector<std::string> fixedDims;
  for (auto dim : mParameterSpace->getDimensions()) {
    if (mParameterSpace->isFilesystemDimension(dim->getName())) {
      filesystemDims.push_back(dim->getName());
    }
    if (std::find(sliceDimensions.begin(), sliceDimensions.end(),
                  dim->getName()) == sliceDimensions.end()) {
      fixedDims.push_back(dim->getName());
    }
  }
  // FIXME implement slicing along more than one dimension.
  std::vector<double> values;

  size_t fieldSize = 1; // FIXME get actual field size
  size_t dimCount = fieldSize;
  for (const auto &sliceDimension : sliceDimensions) {
    auto dim = mParameterSpace->getDimension(sliceDimension);
    if (dim && dim->getSpaceStride() > 0) {
      dimCount *= dim->size() / dim->getSpaceStride();
    } else {
      std::cerr << "ERROR: Unknown dimension: " << sliceDimension << std::endl;
      return std::string();
    }
  }
  //  values.reserve(dimCount);

  std::string filename = "slice_" + field + "_";
  // TODO check if file exists and is the correct slice to use cache instead.
  // TODO for this we need to add metadata to the file indicating where the
  // slice came from. This is part of the bigger TINC metadata idea
  for (const auto &sliceDimension : sliceDimensions) {
    auto dim = mParameterSpace->getDimension(sliceDimension);
    assert(dim);
    if (std::find(filesystemDims.begin(), filesystemDims.end(),
                  sliceDimension) != filesystemDims.end()) {
      // TODO should we perform a  copy of the parameter space to avoid race
      // conditions?
      // sliceDimension is a dimension that affects filesystem paths.
      size_t thisDimCount = dim->size();
      if (dim->getSpaceStride() > 1) {
        thisDimCount /= dim->getSpaceStride();
      }
      values.reserve(thisDimCount);

      auto dataPaths = getAllPaths(fixedDims);
      for (const auto &directory : dataPaths) {
        for (const auto &file : mDataFilenames) {
          auto fullPath = al::File::conformDirectory(directory) + file.first;
          if (al::File::isRelativePath(fullPath)) {
            fullPath = mParameterSpace->getRootPath() +
                       al::File::absolutePath(fullPath);
          }
          if (file.second == sliceDimension) {
            // The file contains the slice.
            values.resize(dim->size());
            if (getFieldFromFile(field, fullPath, values.data(),
                                 values.size())) {
              goto storedata; // We need to exit the two loops
            }
          } else {
            double value;
            size_t index =
                mParameterSpace->getDimension(file.second)->getCurrentIndex();

            if (getFieldFromFile(field, fullPath, index, &value)) {
              values.push_back(value);
              break;
            }
          }
        }
      }
    } else { // We can slice the data from a single file
      values.resize(dim->size());
      std::map<std::string, size_t> currentIndeces;
      for (auto dimension : mParameterSpace->getDimensions()) {
        currentIndeces[dimension->getName()] = dimension->getCurrentIndex();
      }
      auto directory =
          al::File::conformDirectory(mParameterSpace->getRootPath()) +
          mParameterSpace->generateRelativeRunPath(currentIndeces,
                                                   mParameterSpace);
      for (const auto &file : mDataFilenames) {
        if (getFieldFromFile(field,
                             al::File::conformDirectory(directory) + file.first,
                             values.data(), values.size())) {
          // TODO support more than one file
          break;
        }
      }
    }
    filename += sliceDimension + "_";
    //    mParameterSpace->getDimension(sliceDimension)->getCurrentId();
  }
storedata:
  for (auto dim : mParameterSpace->getDimensions()) {
    if (std::find(sliceDimensions.begin(), sliceDimensions.end(),
                  dim->getName()) == sliceDimensions.end()) {
      filename +=
          "_" + dim->getName() + "_" + std::to_string(dim->getCurrentIndex());
    }
  }

  filename = ProcessorScript::sanitizeName(filename) + ".nc";
// TODO accommodate more types apart from double
#ifdef TINC_HAS_NETCDF
  int retval, ncid;
  if ((retval = nc_create((mSliceCacheDirectory + filename).c_str(),
                          NC_NETCDF4 | NC_CLOBBER, &ncid))) {
    std::cerr << "Error creating file: " << filename << std::endl;
    filename.clear();
  }
  int x_dimid, varid;
  int dimids[1];
  if ((retval = nc_def_dim(ncid, "values", values.size(), &x_dimid))) {
    filename.clear();
  }
  dimids[0] = x_dimid;

  /* Define the variable.*/
  if ((retval = nc_def_var(ncid, "data", NC_DOUBLE, 1, dimids, &varid))) {
    filename.clear();
  }
  if ((retval = nc_enddef(ncid))) {
    filename.clear();
  }
  if ((retval = nc_put_var_double(ncid, varid, values.data()))) {
    filename.clear();
  }
  if ((retval = nc_close(ncid))) {
    filename.clear();
  }

#else
  std::cerr << " ERROR not implemented" << std::endl;
  filename.clear();
#endif
  return filename;
}

size_t DataPool::readDataSlice(std::string field, std::string sliceDimension,
                               void *data, size_t maxLen) {
  // TODO accommodate more types apart from double
  auto filename = DataPool::createDataSlice(field, sliceDimension);
  if (filename.size() > 0) {
#ifdef TINC_HAS_NETCDF
    int retval, ncid, varid, nattsp;
    size_t lenp;
    nc_type xtypep;
    int ndimsp;
    int dimidsp[32];
    if ((retval = nc_open((mSliceCacheDirectory + filename).c_str(), NC_NETCDF4,
                          &ncid))) {
      std::cerr << "Error opening file: " << filename << std::endl;
    }
    if ((retval = nc_inq_varid(ncid, "data", &varid))) {
      return 0;
    }

    if ((retval = nc_inq_var(ncid, varid, nullptr, &xtypep, &ndimsp, dimidsp,
                             &nattsp))) {
      return 0;
    }
    if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &lenp))) {
      return 0;
    }

    if (maxLen >= lenp) {
      if ((retval = nc_get_var(ncid, varid, data))) {
        return 0;
      }
      if ((retval = nc_close(ncid))) {
        std::cerr << "Error closing file: " << filename << std::endl;
      }
      return lenp;
    }
    if ((retval = nc_close(ncid))) {
      std::cerr << "Error closing file: " << filename << std::endl;
    }
#endif
    return 0; // FIXME finish the edge case
  } else {
    return 0;
  }
}

void DataPool::setCacheDirectory(std::string cacheDirectory, al::Socket *src) {
  cacheDirectory = al::File::conformDirectory(cacheDirectory);
  if (!al::File::exists(cacheDirectory)) {
    if (!al::Dir::make(cacheDirectory)) {
      std::cerr << "ERROR creating directory: " << cacheDirectory << std::endl;
    }
  }
  mSliceCacheDirectory = cacheDirectory;
  modified(src);
}

std::vector<std::string> DataPool::listFields(bool verifyConsistency) {
  std::vector<std::string> fields;

  if (verifyConsistency) {
    auto paths = getAllPaths({});
    for (const auto &path : paths) {

      for (const auto &file : mDataFilenames) {
        auto fullPath = path + file.first;
        // TODO verify consistency
      }
    }
  } else {
    auto currentFiles = getCurrentFiles();
    if (currentFiles.size() > 0) {
      std::string file = currentFiles[0];
      return listFieldInFile(file);
    } else {
      std::cerr << __FILE__ << ":" << __LINE__ << " ERROR field not present"
                << std::endl;
    }
  }

  return fields;
}

std::vector<std::string> DataPool::getCurrentFiles() {
  std::vector<std::string> files;
  std::string path =
      al::File::conformDirectory(mParameterSpace->getRootPath() +
                                 mParameterSpace->getCurrentRelativeRunPath());
  for (const auto &f : mDataFilenames) {
    auto fullPath = path + f.first;
    if (!al::File::isRelativePath(fullPath)) {
      fullPath = al::File::absolutePath(fullPath);
    }
    files.push_back(fullPath);
  }
  return files;
}

std::map<std::string, std::string> DataPool::getRegisteredDataFiles() {
  return mDataFilenames;
}

void DataPool::clearRegisteredFiles() {
  mDataFilenames.clear();
  // TODO send to clients
}
