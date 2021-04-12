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
  bool loadData(std::string filename = "") override;

  /**
   * @brief Write data from memory to disk buffer and then load it.
   * @param newData
   * @return the name of the file where buffer was written
   */
  std::string setData(DataType &newData);

  void registerUpdateCallback(std::function<void(bool)> cb) {
    mUpdateCallbacks.push_back(cb);
  }

protected:
  virtual bool parseFile(std::string fileName,
                         std::shared_ptr<DataType> newData) = 0;

  virtual bool encodeData(std::string fileName, DataType &newData) = 0;

  std::vector<std::function<void(bool)>> mUpdateCallbacks;
};

// ----------------------------------
template <class DataType>
DiskBuffer<DataType>::DiskBuffer(std::string id, std::string fileName,
                                 std::string path, uint16_t size)
    : BufferManager<DataType>(size) {
  mId = id;
  // TODO there should be a check through TincProtocol to make sure names are
  // unique
  m_baseFileName = fileName;
  if (path.size() > 0) {
    m_path = al::File::conformDirectory(path);
  } else {
    m_path = "";
  }
}

template <class DataType>
bool DiskBuffer<DataType>::loadData(std::string filename) {
  std::unique_lock<std::mutex> lk(m_writeLock);
  if (filename.size() > 0) {
    m_fileName = filename;
  }
  std::shared_ptr<DataType> newData = BufferManager<DataType>::getWritable();
  auto ret = parseFile(filename, newData);
  if (ret) {
    BufferManager<DataType>::doneWriting(newData);
  }
  for (auto cb : mUpdateCallbacks) {
    cb(ret);
  }
  return ret;
}

template <class DataType>
std::string DiskBuffer<DataType>::setData(DataType &newData) {
  std::unique_lock<std::mutex> lk(m_writeLock);
  auto fileName = getFilenameForWriting();
  if (encodeData(fileName, newData)) {
    lk.unlock();
    if (!loadData(fileName)) {
      std::cerr << __FILE__ << ":" << __LINE__
                << " ERROR: DiskBuffer failed to load written data."
                << std::endl;
      return std::string();
    }
    return fileName;
  }
  return std::string();
}

} // namespace tinc

#endif // DISKBUFFER_HPP
