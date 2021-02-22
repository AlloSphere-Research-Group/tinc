#ifndef DISKBUFFER_HPP
#define DISKBUFFER_HPP

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

#include <cstring>
#include <errno.h>
#include <fstream>
#include <string>

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/BufferManager.hpp"
#include "tinc/DiskBufferAbstract.hpp"

namespace tinc {

template <class DataType>
class DiskBuffer : public BufferManager<DataType>, public DiskBufferAbstract {
public:
  DiskBuffer(std::string id = "", std::string fileName = "",
             std::string path = "", uint16_t size = 2);
  /**
   * @brief updateData
   * @param filename
   * @return
   *
   * Whenever overriding this function, you must make sure you call the
   * update callbacks in mUpdateCallbacks
   */
  bool updateData(std::string filename = "") override;

  void registerUpdateCallback(std::function<void(bool)> cb) {
    mUpdateCallbacks.push_back(cb);
  }

protected:
  virtual bool parseFile(std::ifstream &file,
                         std::shared_ptr<DataType> newData) = 0;

  std::vector<std::function<void(bool)>> mUpdateCallbacks;

  // Make this function private as users should not have a way to make the
  // buffer writable. Data writing should be done by writing to the file.
  using BufferManager<DataType>::getWritable;
};

template <class DataType>
DiskBuffer<DataType>::DiskBuffer(std::string id, std::string fileName,
                                 std::string path, uint16_t size)
    : BufferManager<DataType>(size) {
  mId = id;
  // TODO there should be a check through a singleton to make sure names are
  // unique
  m_fileName = fileName;
  if (path.size() > 0) {
    m_path = al::File::conformDirectory(path);
  } else {
    m_path = "";
  }
}

template <class DataType>
bool DiskBuffer<DataType>::updateData(std::string filename) {
  std::unique_lock<std::mutex> lk(mWriteLock);
  if (filename.size() > 0) {
    m_fileName = filename;
  }
  std::ifstream file(m_path + m_fileName);
  bool ret = false;
  if (file.good()) {
    auto buffer = getWritable();
    ret = parseFile(file, buffer);
    BufferManager<DataType>::doneWriting(buffer);
  } else {
    std::cerr << "Error code: " << std::strerror(errno);
  }
  for (auto cb : mUpdateCallbacks) {
    cb(ret);
  }
  return ret;
}

} // namespace tinc

#endif // DISKBUFFER_HPP
