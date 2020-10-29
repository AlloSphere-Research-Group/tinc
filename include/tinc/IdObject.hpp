#ifndef IDOBJECT_HPP
#define IDOBJECT_HPP

#include <string>
#include <iostream>
#include <functional>

namespace tinc {

class IdObject {
public:
  std::string getId();

  void setId(std::string id);

  std::function<void()> modified = []() {};

protected:
  std::string mId;
};
}

#endif // IDOBJECT_HPP
