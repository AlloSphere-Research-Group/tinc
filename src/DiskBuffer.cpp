#include "tinc/DiskBuffer.hpp"

using namespace tinc;

void AbstractDiskBuffer::exposeToNetwork(al::ParameterServer &p) {
  if (m_trigger) {
    std::cerr << "ERROR: already registered. Aborting." << std::endl;
    return;
  }
  std::string pathPrefix = "/__DiskBuffer/";
  //    if (m_fileName[0] != '/') {
  //      pathPrefix += "/";
  //    }
  m_trigger = std::make_shared<al::ParameterString>(pathPrefix + id);
  p.registerParameter(*m_trigger);
  m_trigger->registerChangeCallback(
      [this](std::string value) { this->updateData(value); });
  // There will be problems if this object is destroyed before the parameter
  // server Should this be a concern?
}
