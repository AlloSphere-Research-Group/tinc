#ifndef DISKBUFFERTEXT_HPP
#define DISKBUFFERTEXT_HPP

/*
 * Copyright 2020-2021 AlloSphere Research Group
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

#ifdef TINC_CPP_17
#include <filesystem>
#else
#ifdef AL_WINDOWS
#include "Shlwapi.h"
#endif
#endif
#include <fstream>
#include <streambuf>
#include <string>

namespace tinc {
/**
 * @brief A DiskBuffer that reads and decodes text files.
 */
class DiskBufferText : public DiskBuffer<std::string> {
public:
  DiskBufferText(std::string id, std::string fileName = "",
                 std::string relPath = "", std::string rootPath = "",
                 uint16_t size = 2)
      : DiskBuffer<std::string>(id, fileName, relPath, rootPath, size) {}

protected:
  bool parseFile(std::string fileName,
                 std::shared_ptr<std::string> newData) override;

  bool encodeData(std::string fileName, std::string &newData) override;
};

inline bool DiskBufferText::parseFile(std::string fileName,
                                      std::shared_ptr<std::string> newData) {
  //    bool ret = false;
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
  std::ifstream t(filePath);

  t.seekg(0, std::ios::end);
  newData->reserve(t.tellg());
  t.seekg(0, std::ios::beg);
  // TODO check if file was opened correctlt

  newData->assign((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

  BufferManager<std::string>::doneWriting(newData);
  //      ret = true;
  //    } else {
  //      std::cerr << "Error reading Image: " << filePath << std::endl;
  //    }
  return true;
}

inline bool DiskBufferText::encodeData(std::string fileName,
                                       std::string &newData) {
  //    bool ret = false;
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
  std::ofstream t(filePath);
  // TODO test if writing went OK
  t << newData;
  t.close();
  return true;
  //    if (newData.save(filePath)) {
  //      ret = true;
  //    } else {
  //      std::cerr << "Error writing Image: " << filePath << std::endl;
  //    }
  //    return ret;
}

} // namespace tinc

#endif // DISKBUFFERTEXT_HPP
