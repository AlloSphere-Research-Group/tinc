#include "tinc/DataPool.hpp"

#include "al/io/al_File.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#ifdef TINC_HAS_NETCDF
#include <netcdf.h>
#endif

#include <fstream>

using namespace tinc;

DataPool::DataPool(ParameterSpace &ps, std::string sliceCacheDir)
    : mParameterSpace(&ps),
      mSliceCacheDirectory(al::File::conformDirectory(sliceCacheDir)) {}

std::string DataPool::createDataSlice(std::string field,
                                      std::string sliceDimension) {
  return createDataSlice(field, std::vector<std::string>{sliceDimension});
}

std::string
DataPool::createDataSlice(std::string field,
                          std::vector<std::string> sliceDimensions) {
  auto filesystemDims = mParameterSpace->dimensionsForFilesystem();
  // TODO implement slicing along more than one dimension.
  std::vector<float> values;
  std::string filename = "slice_";

  size_t fieldSize = 1; // TODO get actual field size
  size_t dimCount = fieldSize;
  for (auto sliceDimension : sliceDimensions) {
    auto dim = mParameterSpace->getDimension(sliceDimension);
    if (dim) {
      dimCount *= dim->size();
    } else {
      std::cerr << "ERROR: Unknown dimension: " << sliceDimension << std::endl;
      return std::string();
    }
  }
  values.reserve(dimCount);

  // TODO check if file exists and is the correct slice to use cache instead.
  // TODO for this we need to add metadata to the file indicating where the
  // slice came from. This is part of the bigger TINC metadata idea
  for (auto sliceDimension : sliceDimensions) {
    auto dim = mParameterSpace->getDimension(sliceDimension);
    assert(dim);
    if (std::find(filesystemDims.begin(), filesystemDims.end(),
                  sliceDimension) == filesystemDims.end()) {
      // TODO should we perform a  copy of the parameter space to avoid race
      // conditions?
      dim->lock();
      // sliceDimension is a dimension that affects filesystem paths.
      size_t dimCount = dim->size();
      values.reserve(dimCount);
      auto dataPaths = mParameterSpace->runningPaths();
      for (auto directory : dataPaths) {
        for (auto file : mDataFilenames) {
          float value;
          size_t index =
              mParameterSpace->getDimension(file.second)->getCurrentIndex();
          if (getFieldFromFile(field,
                               directory + AL_FILE_DELIMITER + file.first,
                               index, &value)) {
            values.push_back(value);
            break;
          }
        }
      }

      dim->unlock();
    } else { // We can slice the data from a single file
      values.resize(dim->size());
      std::map<std::string, size_t> currentIndeces;
      for (auto dimension : mParameterSpace->dimensions) {
        currentIndeces[dimension->getName()] = dimension->getCurrentIndex();
      }
      auto directory =
          al::File::conformPathToOS(mParameterSpace->generateRelativeRunPath(
              currentIndeces, mParameterSpace));
      for (auto file : mDataFilenames) {
        if (getFieldFromFile(field, directory + file.first, values.data(),
                             values.size())) {
          break;
        }
      }
    }
    filename += sliceDimension + +"_" +
                mParameterSpace->getDimension(sliceDimension)->getCurrentId();
  }

  for (auto dim : mParameterSpace->dimensions) {
    if (std::find(sliceDimensions.begin(), sliceDimensions.end(),
                  dim->getName()) == sliceDimensions.end()) {
      filename += "_" + dim->getName();
    }
  }
  filename += ".nc";
#ifdef TINC_HAS_NETCDF
  int retval, ncid;
  if ((retval = nc_create(filename.c_str(), NC_NETCDF4 | NC_CLOBBER, &ncid))) {
    std::cerr << "Error opening file: " << filename << std::endl;
    filename.clear();
  }
  int x_dimid, varid;
  int dimids[1];
  if ((retval = nc_def_dim(ncid, "values", values.size(), &x_dimid))) {
    filename.clear();
  }
  dimids[0] = x_dimid;

  /* Define the variable.*/
  if ((retval = nc_def_var(ncid, "data", NC_FLOAT, 1, dimids, &varid))) {
    filename.clear();
  }
  if ((retval = nc_enddef(ncid))) {
    filename.clear();
  }
  if ((retval = nc_put_var_float(ncid, varid, values.data()))) {
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
  auto filename = DataPool::createDataSlice(field, sliceDimension);
  if (filename.size() > 0) {
#ifdef TINC_HAS_NETCDF
    int retval, ncid, varid, nattsp;
    size_t lenp;
    nc_type xtypep;
    int ndimsp;
    int dimidsp[32];
    if ((retval = nc_open(filename.c_str(), NC_NETCDF4, &ncid))) {
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
      return lenp;
    }
#endif

  } else {
    return 0;
  }
}

bool DataPool::getFieldFromFile(std::string field, std::string file,
                                size_t dimensionInFileIndex, void *data) {
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return false;
  }
  json j = json::parse(f);
  auto fieldData = j[field].at(dimensionInFileIndex).get<float>();
  *(float *)data = fieldData;
  return true;
}

bool DataPool::getFieldFromFile(std::string field, std::string file, void *data,
                                size_t length) {
  std::ifstream f(file);
  if (!f.good()) {
    std::cerr << "ERROR reading file: " << file << std::endl;
    return false;
  }
  json j;
  f >> j;
  auto fieldData = j[field];
  if (fieldData && fieldData.is_array()) {
    memcpy((float *)data, fieldData.get<std::vector<float>>().data(),
           length * sizeof(float));
  } else {
    return false;
  }
  return true;
}
