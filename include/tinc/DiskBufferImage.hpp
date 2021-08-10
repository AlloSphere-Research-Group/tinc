#ifndef DISKBUFFERIMAGE_HPP
#define DISKBUFFERIMAGE_HPP

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

#include "tinc/DiskBuffer.hpp"

#include "al/graphics/al_Image.hpp"

#ifdef TINC_CPP_17
#include <filesystem>
#else
#ifdef AL_WINDOWS
#include "Shlwapi.h"
#endif
#endif

namespace tinc {

class DiskBufferImage : public DiskBuffer<al::Image> {
public:
  DiskBufferImage(std::string id, std::string fileName = "",
                  std::string relPath = "", std::string rootPath = "",
                  uint16_t size = 2)
      : DiskBuffer<al::Image>(id, fileName, relPath, rootPath, size) {}

  bool writePixels(unsigned char *newData, int width, int height,
                   int numComponents = 3, std::string filename = "") {

    if (filename.size() == 0) {
      filename = getFilenameForWriting();
    }
    if (!al::Image::saveImage(getFullPath() + filename, newData, width, height,
                              false, numComponents)) {
      std::cerr << __FILE__ << ":" << __LINE__
                << " ERROR writing image file: " << getFullPath() + filename
                << std::endl;
    }

    return loadData(filename);
  };

protected:
  bool parseFile(std::string fileName,
                 std::shared_ptr<al::Image> newData) override {
    bool ret = false;
    std::string filePath;

#ifdef TINC_CPP_17
    if (std::filesystem::path(fileName).is_absolute()) {
#else

#ifdef AL_WINDOWS
    if (!PathIsRelative(fileName.c_str())) {
#else
    if (fileName.size() > 0 && fileName[0] == '/') {
#endif
#endif
      filePath = fileName;
    } else {
      filePath = getFullPath() + fileName;
    }
    if (newData->load(filePath)) {
      BufferManager<al::Image>::doneWriting(newData);
      ret = true;
    } else {
      std::cerr << "Error reading Image: " << filePath << std::endl;
    }
    return true;
  }

  bool encodeData(std::string fileName, al::Image &newData) override {
    bool ret = false;
    std::string filePath;
#ifdef TINC_CPP_17
    if (std::filesystem::path(fileName).is_absolute()) {
#else

#ifdef AL_WINDOWS
    if (!PathIsRelative(fileName.c_str())) {
#else
    if (fileName.size() > 0 && fileName[0] == '/') {
#endif
#endif
      filePath = fileName;
    } else {
      filePath = getFullPath() + fileName;
    }
    if (newData.save(filePath)) {
      ret = true;
    } else {
      std::cerr << "Error writing Image: " << filePath << std::endl;
    }
    return ret;
  }
};

} // namespace tinc

#endif // DISKBUFFERIMAGE_HPP
