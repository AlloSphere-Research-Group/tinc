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

  ParameterSpace &getParameterSpace() { return *mParameterSpace; }

  std::string createDataSlice(std::string field, std::string sliceDimension);
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

  std::string getCacheDirectory() { return mSliceCacheDirectory; }

  void setCacheDirectory(std::string cacheDirectory);

  /** Replace this function when the parameter space path function is not
  * adequate.
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
