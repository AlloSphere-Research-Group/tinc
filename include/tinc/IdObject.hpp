#ifndef IDOBJECT_HPP
#define IDOBJECT_HPP

#include <string>
#include <iostream>

namespace tinc {

class IdObject {
public:
  std::string getId();

  void setId(std::string id);

protected:
  std::string mId;
};
}

#endif // IDOBJECT_HPP
