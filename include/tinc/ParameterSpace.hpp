#ifndef PARAMETERSPACE_HPP
#define PARAMETERSPACE_HPP

#include "tinc/ParameterSpaceDimension.hpp"
#include "tinc/Processor.hpp"

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace tinc {

class ParameterSpace {
public:
  ParameterSpace();

  ~ParameterSpace();

  std::shared_ptr<ParameterSpaceDimension> getDimension(std::string name);

  void registerDimension(std::shared_ptr<ParameterSpaceDimension> dimension);

  /**
   * @brief Returns all the paths that are used by the whole parameter space
   */
  std::vector<std::string> runningPaths();

  /**
   * @brief Get relative filesystem path for current parameter values
   * @return
   *
   * Generated according to generateRelativePath()
   */
  std::string currentRunPath();

  /**
   * @brief Returns the names of all dimensions
   */
  std::vector<std::string> dimensionNames();

  /**
   * @brief Returns the names of all dimensions that affect filesystem
   * directories
   * @return
   *
   * Only mappedParameters and conditions are considered. Regular parameters
   * are considered not to affect filesystem location
   */
  std::vector<std::string> dimensionsForFilesystem();

  /**
   * @brief removes all dimensions from parameter space
   */
  void clear();

  /**
   * @brief increment to next index from index array
   * @param currentIndeces
   * @return true when no more indeces to process
   */
  bool incrementIndeces(std::map<std::string, size_t> &currentIndeces);

  /**
   * @brief sweep the parameter space across all or specified dimensions
   * @param processor processor to sweep with parameter space
   * @param dimensionNames names of dimensions to sweep, all if empty
   * @param recompute force recompute if true
   */
  void sweep(Processor &processor, std::vector<std::string> dimensionNames = {},
             bool recompute = false);

  void sweepAsync(Processor &processor,
                  std::vector<std::string> dimensionNames = {},
                  bool recompute = false);

  void stopSweep();

  /**
   * @brief Create necessary filesystem directories to be populated by data
   * @return true if successfully created (or checked existence) of
   * directories.
   */
  bool createDataDirectories();

  /**
   * @brief Load parameter space dimensions from disk file
   * @param ncFile
   * @return
   *
   * The file is loaded relative to 'rootPath'. Dimension found in the file are
   * added to the current parameter space if a dimension with that name already
   * exists, it is replaced.
   */
  bool readFromNetCDF(std::string ncFile = "parameter_space.nc");

  /**
   * @brief write parameter sapce dimensions to netCDF file.
   * @param fileName
   * @return
   */
  bool writeToNetCDF(std::string fileName = "parameter_space.nc");

  /**
   * @brief Read dimensions from parameter space netcdf file
   * @param filename
   * @param[out] newDimensions
   * @return true if read was succesful
   */
  bool readDimensionsInNetCDFFile(
      std::string filename,
      std::vector<std::shared_ptr<ParameterSpaceDimension>> &newDimensions);

  /**
   * @brief Filesystem root path for parameter space
   *
   * This root path is where the root parameter space should be localted and
   * should contain all data directories
   */
  std::string rootPath;

  // These should not be modifed by the user (perhaps make private?)
  std::vector<std::shared_ptr<ParameterSpaceDimension>> dimensions;

  // To map names provided to getDimension() to internal data names
  // You can also use this map to display user friendly names when displaying
  // parameters
  std::map<std::string, std::string> parameterNameMap;

  // FIXME this interface is a little uncomfortable and not consistent with
  // the other similar functionality in setOuputFilename(), perhaps an output
  // filename generator function should be provided?
  std::function<std::string(std::map<std::string, size_t>)>
      generateRelativeRunPath = [&](std::map<std::string, size_t> indeces) {
        std::string path;
        for (auto dimensionSample : indeces) {
          std::shared_ptr<ParameterSpaceDimension> dimension;

          for (auto ps : dimensions) {
            if (ps->parameter().getName() == dimensionSample.first) {
              dimension = ps;
              break;
            }
          }
          if (dimension) {
            auto id = dimension->idAt(dimensionSample.second);
            path += id + "/";
          }
        }
        return path;
      };
  std::function<std::vector<std::string>(std::map<std::string, size_t>)>
      generateOutpuFileNames;

  /**
   * @brief onSweepProcess is called after a sample completes processing as part
   * of a sweep
   */
  std::function<void(std::map<std::string, size_t> currentIndeces,
                     double progress)> onSweepProcess;

  /**
   * @brief callback when the value in any particular dimension changes.
   *
   * You should use this callback whenever you need to know the specific
   * dimension that has changed. For this reason, you should not update
   * processor configurations from here, as this function will not be called,
   * except when a particualr dimension has changed
   */
  // FIXME we need to account for other parameter types not only float
  std::function<void(float oldValue, ParameterSpaceDimension *changedDimension)>
      onValueChange = [](float /*oldValue*/,
                         ParameterSpaceDimension * /*changedDimension*/) {};

  std::function<void()> updateConfiguration = []() {};

protected:
  /**
 * @brief update current position to value in dimension ps
 * @param oldValue You should pass the previous value here.
 * @param ps
 *
 * This function checks if new dataset directory needs a reload of
 * parameter_space.nc and processes the parameter space changes
 */
  void updateParameterSpace(float oldValue, ParameterSpaceDimension *ps);

  std::unique_ptr<std::thread> mAsyncProcessingThread;

  bool mSweepRunning{false};

  // Subdirectories that have a parameter space file in them.
  std::map<std::string, std::string> mSpecialDirs;

private:
};
} // namespace tinc

#endif // PARAMETERSPACE_HPP
