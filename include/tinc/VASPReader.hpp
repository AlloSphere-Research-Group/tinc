#ifndef VASPREADER_HPP
#define VASPREADER_HPP

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

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "al/math/al_Mat.hpp"
#include "al/math/al_Vec.hpp"

namespace tinc {

/**
 * @brief Simple class to read VASP files.
 */
class VASPReader {
public:
  typedef enum {
    USE_INLINE_ELEMENT_NAMES,  // Set positions using inline names (the name
                               // accompanying the position is used instead of
                               // following the species declaration line)
    DONT_VALIDATE_INLINE_NAMES //
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

  std::map<std::string, std::vector<float>> &
  getAllPositions(bool transform = true);

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

} // namespace tinc

#endif // VASPREADER_HPP
