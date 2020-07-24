#ifndef DISKBUFFER_HPP
#define DISKBUFFER_HPP

#include <fstream>
#include <string>

#include "al/io/al_File.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_ParameterServer.hpp"

#include "tinc/BufferManager.hpp"

namespace tinc {

class AbstractDiskBuffer {
public:
  std::string id;

  // Careful, this is not thread safe. Needs to be called synchronously to any
  // process functions
  std::string getCurrentFileName() { return m_fileName; }

  virtual bool updateData(std::string filename) = 0;
  void exposeToNetwork(al::ParameterServer &p);

protected:
  std::string m_fileName;
  std::string m_path;
  std::shared_ptr<al::ParameterString> m_trigger;
};

template <class DataType>
class DiskBuffer : public BufferManager<DataType>, public AbstractDiskBuffer {
public:
  DiskBuffer(std::string name, std::string fileName = "", std::string path = "",
             uint16_t size = 2);

  bool updateData(std::string filename = "") override;

protected:
  virtual bool parseFile(std::ifstream &file,
                         std::shared_ptr<DataType> newData) = 0;

  // Make this function private as users should not have a way to make the
  // buffer writable. Data writing should be done by writing to the file.
  using BufferManager<DataType>::getWritable;
};

template <class DataType>
DiskBuffer<DataType>::DiskBuffer(std::string name, std::string fileName,
                                 std::string path, uint16_t size)
    : BufferManager<DataType>(size) {
  id = name;
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
  if (file.good()) {
    auto buffer = getWritable();
    bool ret = parseFile(file, buffer);
    BufferManager<DataType>::doneWriting(buffer);
    return ret;
  } else {
    std::cerr << "Error code: " << strerror(errno);
    return false;
  }
}

} // namespace tinc

#endif // DISKBUFFER_HPP
