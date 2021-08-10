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

#include "tinc/DistributedPath.hpp"
#include "tinc/IdObject.hpp"

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"

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
   * @param notify if true, the notification callbacks are called
   * @return true if succesfully loaded file
   *
   * You should normally call this function without setting the notify argument.
   */
  virtual bool loadData(std::string filename, bool notify = true) = 0;

  std::string getBaseFileName() { return m_distPath.filename; }
  void setBaseFileName(std::string name) { m_distPath.filename = name; }

  void setRelativePath(std::string path) { m_distPath.relativePath = path; }
  std::string getRelativePath() { return m_distPath.relativePath; }

  void setRootPath(std::string path) { m_distPath.rootPath = path; }
  std::string getRootPath() { return m_distPath.rootPath; }

  std::string getFullPath() { return m_distPath.path(); }

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

  void registerUpdateCallback(std::function<void(bool)> cb) {
    mUpdateCallbacks.push_back(cb);
  }

  /**
   * @brief register a function to be called when this object generates new
   * buffer data
   * @param cb
   *
   * If this object is registered with a TINC server or client, this function
   * will only be called if the data creation is local to the node.
   */
  void registerNotifyCallback(std::function<void(bool)> cb) {
    mNotifyCallbacks.push_back(cb);
  }

protected:
  std::string m_fileName;

  DistributedPath m_distPath;

  std::shared_ptr<al::ParameterString> m_trigger;

  std::mutex m_writeLock;

  uint64_t m_roundRobinCounter{0};
  uint64_t m_roundRobinSize{0};

  std::vector<std::function<void(bool)>> mUpdateCallbacks;
  std::vector<std::function<void(bool)>> mNotifyCallbacks;

  std::string makeNextFileName() {
    std::string outName = m_distPath.filename;
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
    auto ext = al::File::extension(m_distPath.filename);
    std::string newName;
    if (ext.size() > 0) {
      newName = m_distPath.filename.substr(0, m_distPath.filename.size() -
                                                  ext.size());
    } else {
      newName = m_distPath.filename;
    }
    newName += "_" + std::to_string(index) + ext;
    return newName;
  }
};

} // namespace tinc

#endif // DISKBUFFERABSTRACT_HPP
