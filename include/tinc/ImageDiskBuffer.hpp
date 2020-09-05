#ifndef IMAGEDISKBUFFER_HPP
#define IMAGEDISKBUFFER_HPP

#include "tinc/DiskBuffer.hpp"

#include "al/graphics/al_Image.hpp"

namespace tinc {

class ImageDiskBuffer : public DiskBuffer<al::Image> {
public:
  ImageDiskBuffer(std::string id, std::string fileName = "",
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

protected:
  bool parseFile(std::ifstream &file,
                 std::shared_ptr<al::Image> newData) override {
    return true;
  }
};

} // namespace tinc

#endif // IMAGEDISKBUFFER_HPP
