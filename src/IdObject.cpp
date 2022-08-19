#include "tinc/IdObject.hpp"

#include "al/io/al_File.hpp"

using namespace tinc;

// IdObject::IdObject() {}

std::string IdObject::getId() {
  if (mId.size() == 0) {
    mId = al::demangle(typeid(*this).name()) + "@" +
          std::to_string((uint64_t)this);
    std::cout << "Warning. Processor given temporary id: " << mId << std::endl;
  }
  return mId;
}

void IdObject::setId(std::string id) {
  if (mId.size() == 0) {
    mId = id;
  } else {
    std::cerr << "ERROR, id for DataPool already exists. Not set." << std::endl;
  }
}

std::string IdObject::getDocumentation() const { return mDocumentation; }

void IdObject::setDocumentation(const std::string &documentation,
                                al::Socket *src) {
  mDocumentation = documentation;
  modified(src);
}

void IdObject::setVerbose(bool set) { mVerbose = set; }
