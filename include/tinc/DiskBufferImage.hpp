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

namespace tinc {

class DiskBufferImage : public DiskBuffer<al::Image> {
public:
  DiskBufferImage(std::string id, std::string fileName = "",
                  std::string path = "", uint16_t size = 2)
      : DiskBuffer<al::Image>(id, fileName, path, size) {}

  bool updateData(std::string filename = "") override {
    if (filename.size() > 0) {
      m_fileName = filename;
    }
    auto buffer = getWritable();
    bool ret = false;
    if (buffer->load(m_path + m_fileName)) {
      BufferManager<al::Image>::doneWriting(buffer);
      ret = true;
    } else {
      std::cerr << "Error reading Image: " << m_path + m_fileName << std::endl;
    }
    for (auto cb : mUpdateCallbacks) {
      cb(ret);
    }
    return ret;
  }

  bool writePixels(unsigned char *newData, int width, int height,
                   std::string filename = "") {

    if (filename.size() == 0) {
      filename = getCurrentFileName();
    }
    al::Image::saveImage(filename, newData, width, height);

    return updateData(filename);
  };

protected:
  bool parseFile(std::ifstream &file,
                 std::shared_ptr<al::Image> newData) override {
    // TODO implement
    return true;
  }
};

} // namespace tinc

#endif // DISKBUFFERIMAGE_HPP
