#ifndef DISKBUFFERJSON_HPP
#define DISKBUFFERJSON_HPP

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

#include "tinc/DiskBuffer.hpp"

#include "nlohmann/json.hpp"

namespace tinc {

class DiskBufferJson : public DiskBuffer<nlohmann::json> {
public:
  DiskBufferJson(std::string id = "", std::string fileName = "",
                 std::string relPath = "", std::string rootPath = "",
                 uint16_t size = 2)
      : DiskBuffer<nlohmann::json>(id, fileName, relPath, rootPath, size) {}

protected:
  bool parseFile(std::string fileName,
                 std::shared_ptr<nlohmann::json> newData) override {
    bool ret = false;
    try {
      std::ifstream file(getFullPath() + fileName);
      if (file.good()) {
        //          std::cout << "file" << std::endl;
        *newData = nlohmann::json::parse(file);
        ret = true;
      }
    } catch (std::exception & /*e*/) {
      std::cerr << "ERROR: parsing json file " << getFullPath() + fileName
                << std::endl;
      ret = false;
    }
    return ret;
  }

  bool encodeData(std::string fileName, nlohmann::json &newData) override {
    std::ofstream of(getFullPath() + fileName, std::ofstream::out);
    if (of.good()) {
      of << newData.dump(2);
      of.close();
      if (!of.good()) {
        std::cout << "Error writing json file." << getFullPath() + fileName
                  << std::endl;
        return false;
      }
    } else {
      std::cout << "Error creating json file" << getFullPath() + fileName
                << std::endl;
      return false;
    }

    return true;
  }
};

} // namespace tinc

#endif // DISKBUFFERJSON_HPP
