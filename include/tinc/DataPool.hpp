#ifndef DATAPOOL_HPP
#define DATAPOOL_HPP

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

protected:
  bool getFieldFromFile(std::string field, std::string file,
                        size_t dimensionInFileIndex, void *data);
  bool getFieldFromFile(std::string field, std::string file, void *data,
                        size_t length);

private:
  std::string mId;
  ParameterSpace *mParameterSpace;
  std::string mSliceCacheDirectory;
  std::map<std::string, std::string> mDataFilenames;
};
}

#endif // DATAPOT_HPP
