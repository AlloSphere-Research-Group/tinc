#ifndef DATAPOOL_HPP
#define DATAPOOL_HPP

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

#include "tinc/ParameterSpace.hpp"

namespace tinc {
/**
 * @brief The DataPool class gathers data files across directories that span a
 * parameter space
 *
 * The parameter space and its current values determine where the files in the
 * data pool are found. This class is useful to manage data files that are the
 * result of parameter sweeps, generating the same type of file in different
 * directories, where each directory represents a sample of the parameter space.
 */
class DataPool : public IdObject {
public:
  DataPool(std::string id, ParameterSpace &ps,
           std::string sliceCacheDir = std::string());

  DataPool(ParameterSpace &ps, std::string sliceCacheDir = std::string());

  /**
   * @brief registerDataFile
   * @param filename
   *
   * This filename must be relative to mParameterSpace->generateRelativeRunPath
   */
  void registerDataFile(std::string filename, std::string dimensionInFile) {
    mDataFilenames[filename] = dimensionInFile;
  }

  /**
   * @brief Get parameter space that controls this data pool
   * @return the parameter space
   */
  ParameterSpace &getParameterSpace() { return *mParameterSpace; }

  /**
   * @brief Extract a slice of data
   * @param field name of the field to extract
   * @param sliceDimension dimension that can change across the slice
   * @return filename of the extracted slice
   *
   * The slice will be created as a NetCDF4 file with a single variable called
   * "data" that spans a dimension "values". The result will be a one
   * dimensional slice containing the values of the "field" across all values
   * for "sliceDimension" that must be registered in the parameter space.
   */
  std::string createDataSlice(std::string field, std::string sliceDimension);

  /**
   * @brief Extract a slice of data
   * @param field name of the field to extract
   * @param sliceDimensions dimensions that can change across the slice
   * @return filename of the extracted slice
   *
   * The output is a multidimensional slice of the data for the "field" values.
   * The number of dimensions of the result is the size of sliceDimensions
   */
  // FIXME implement multidimensional slicing
  std::string createDataSlice(std::string field,
                              std::vector<std::string> sliceDimensions);

  /**
   * @brief get list of full path to current files in datapool
   * @return list of files
   *
   * Both root path and current run path are prepended to the file names.
   */
  std::vector<std::string> getCurrentFiles();

  /**
   * @brief readDataSlice
   * @param field
   * @param sliceDimension
   * @param data
   * @param maxLen
   * @return number of elements written to data pointer
   */
  size_t readDataSlice(std::string field, std::string sliceDimension,
                       void *data, size_t maxLen);

  /**
   * @brief getCacheDirectory
   * @return cache directory
   */
  std::string getCacheDirectory() { return mSliceCacheDirectory; }

  void setCacheDirectory(std::string cacheDirectory);

  /**
   *  Replace this function when the parameter space runningPaths() function is
   * not adequate.
   */
  std::function<std::vector<std::string>()> getAllPaths = [&]() {
    return mParameterSpace->runningPaths();
  };

protected:
  bool getFieldFromFile(std::string field, std::string file,
                        size_t dimensionInFileIndex, void *data);
  bool getFieldFromFile(std::string field, std::string file, void *data,
                        size_t length);

  std::string getFileType(std::string file);

private:
  ParameterSpace *mParameterSpace;
  std::string mSliceCacheDirectory;
  std::map<std::string, std::string> mDataFilenames;
};
}

#endif // DATAPOT_HPP
