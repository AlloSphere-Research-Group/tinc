#ifndef AL_VASPREADER
#define AL_VASPREADER

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "al/math/al_Mat.hpp"
#include "al/math/al_Vec.hpp"

namespace al {

class VASPReader {
 public:
  typedef enum {
    USE_INLINE_ELEMENT_NAMES,   // Set positions using inline names (the name
                                // accompanying the position is used instead of
                                // following the species declaration line)
    DONT_VALIDATE_INLINE_NAMES  //
  } VASPOption;

  typedef enum {
    VASP_MODE_DIRECT,
    VASP_MODE_CARTESIAN,
    VASP_MODE_NONE
  } VASPMode;

  // Made public to allow access to them
  double mNorm{0.0};
  double maxX = 0, maxY = 0, maxZ = 0;
  double minX = 0, minY = 0, minZ = 0;

  VASPReader(std::string basePath = std::string());

  void ignoreElements(std::vector<std::string> elementsToIgnore);

  void setBasePath(std::string path);

  bool loadFile(std::string fileName);

  std::map<std::string, std::vector<float>> &getAllPositions(
      bool transform = true);

  bool hasElement(std::string elementType);

  void stackCells(int count);

  std::vector<float> &getElementPositions(std::string elementType,
                                          bool transform = true);

  void setOption(VASPOption option, bool enable = true);

  void print();

  al::Vec3d getNormalizingVector();
  al::Vec3d getCenteringVector();

 private:
  std::string mBasePath;
  std::string mFileName;
  bool mVerbose{true};

  VASPMode mMode{VASP_MODE_NONE};
  std::map<std::string, std::vector<float>> mPositions;
  std::vector<std::string> mElementsToIgnore;

  al::Mat3d mTransformMatrix;
  std::mutex mDataLock;
  std::vector<VASPOption> mOptions;
};

}  // namespace al

#endif  // AL_VASPREADER
