#ifndef DISKBUFFERABSTRACT_HPP
#define DISKBUFFERABSTRACT_HPP

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

#include "al/ui/al_Parameter.hpp"
#include "tinc/IdObject.hpp"

#include "al/io/al_File.hpp"

#include <string>

namespace tinc {

/**
 * @brief Base pure virtual class that defines the DiskBuffer interface
 */
class DiskBufferAbstract : public IdObject {
public:
  // Careful, this is not thread safe. Needs to be called synchronously to any
  // process functions
  std::string getCurrentFileName() {
    std::unique_lock<std::mutex> lk(m_writeLock);
    return m_fileName;
  }

  /**
   * @brief Update buffer from file
   * @param filename
   * @return true if succesfully loaded file
   */
  virtual bool loadData(std::string filename) = 0;

  std::string getBaseFileName() { return m_baseFileName; }
  void setBaseFileName(std::string name) { m_baseFileName = name; }

  void setPath(std::string path) { m_path = path; }
  std::string getPath() { return m_path; }

  // TODO implement these
  void cleanupRoundRobinFiles() {}
  void enableRoundRobin(int cacheSize = 0, bool clearLocks = true) {
    m_roundRobinSize = cacheSize;
    // TODO clear file locks
  }

  void useFileLock(bool use = true, bool clearLocks = true) {
    // TODO file locks
  }
  std::string getFilenameForWriting() {
    // TODO check file lock
    std::string outName = makeNextFileName();
    return outName;
  }

protected:
  std::string m_fileName;
  std::string m_baseFileName;
  std::string m_path;
  std::shared_ptr<al::ParameterString> m_trigger;

  std::mutex m_writeLock;

  uint64_t m_roundRobinCounter{0};
  uint64_t m_roundRobinSize{0};

  std::string makeNextFileName() {
    std::string outName = m_baseFileName;
    if (m_roundRobinSize > 0) {
      if (m_roundRobinCounter >= m_roundRobinSize) {
        m_roundRobinCounter = 0;
      }
      outName = makeFileName(m_roundRobinCounter);
      m_roundRobinCounter++;
    }
    return outName;
  }

  std::string makeFileName(uint64_t index) {
    auto ext = al::File::extension(m_baseFileName);
    std::string newName;
    if (ext.size() > 0) {
      newName = m_baseFileName.substr(0, m_baseFileName.size() - ext.size());
    } else {
      newName = m_baseFileName;
    }
    newName += "_" + std::to_string(index) + ext;
    return newName;
  }
};

} // namespace tinc

#endif // DISKBUFFERABSTRACT_HPP
