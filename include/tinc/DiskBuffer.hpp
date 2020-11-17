#ifndef DISKBUFFER_HPP
#define DISKBUFFER_HPP

#include <fstream>
#include <string>
#include <cstring>
#include <errno.h>

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/BufferManager.hpp"
#include "tinc/IdObject.hpp"

namespace tinc {

class AbstractDiskBuffer : public IdObject {
public:
  // Careful, this is not thread safe. Needs to be called synchronously to any
  // process functions
  std::string getCurrentFileName() { return m_fileName; }

  virtual bool updateData(std::string filename) = 0;
  //  void exposeToNetwork(al::ParameterServer &p);

  std::string getBaseFileName() { return m_fileName; }

  void setPath(std::string path) { m_path = path; }
  std::string getPath() { return m_path; }

protected:
  std::string m_fileName;
  std::string m_path;
  std::shared_ptr<al::ParameterString> m_trigger;
};

template <class DataType>
class DiskBuffer : public BufferManager<DataType>, public AbstractDiskBuffer {
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
