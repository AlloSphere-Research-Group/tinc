#ifndef DISKBUFFERNETCDF_HPP
#define DISKBUFFERNETCDF_HPP

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
#include "tinc/VariantValue.hpp"

#ifdef TINC_HAS_NETCDF
#include <netcdf.h>
#endif

#include <map>
#include <vector>

namespace tinc {

/*
#define NC_NAT          0
#define NC_BYTE         1
#define NC_CHAR         2
#define NC_SHORT        3
#define NC_INT          4
#define NC_LONG         NC_INT
#define NC_FLOAT        5
#define NC_DOUBLE       6
#define NC_UBYTE        7
#define NC_USHORT       8
#define NC_UINT         9
#define NC_INT64        10
#define NC_UINT64       11
#define NC_STRING       12
*/

typedef enum {
  NAT = 0,
  BYTE = 1,
  CHAR = 2,
  SHORT = 3,
  INT = 4,
  LONG = INT,
  FLOAT = 5,
  DOUBLE = 6,
  UBYTE = 7,
  USHORT = 8,
  UINT = 9,
  INT64 = 10,
  UINT64 = 11,
  STRING = 12
} NetCDFTypes;

class NetCDFData {
public:
  std::map<std::string, VariantValue> attributes;
  void *dataVector{nullptr};

  ~NetCDFData() {
    if (dataVector) {
      delete dataVector;
    }
  }

  int getType() { return ncDataType; }
  void setType(int type) {
    if (type == ncDataType) {
      return;
    }
    if (dataVector) {
      delete dataVector;
    }
    dataVector = nullptr;
#ifdef TINC_HAS_NETCDF
    switch (type) {
    case NC_SHORT:
      dataVector = new std::vector<int16_t>;
      break;
    case NC_INT:
      dataVector = new std::vector<int32_t>;
      break;
    case NC_FLOAT:
      dataVector = new std::vector<float>;
      break;
    case NC_DOUBLE:
      dataVector = new std::vector<double>;
      break;
    case NC_UBYTE:
    case NC_CHAR:
      dataVector = new std::vector<uint8_t>;
      break;
    case NC_USHORT:
      dataVector = new std::vector<uint16_t>;
      break;
    case NC_UINT:
      dataVector = new std::vector<int32_t>;
      break;
    case NC_INT64:
      dataVector = new std::vector<int64_t>;
      break;
    case NC_UINT64:
      dataVector = new std::vector<uint64_t>;
      break;
    case NC_STRING:
      dataVector = new std::vector<std::string>;

      std::cerr << "string not yet supported in DisnkBufferNetCDF" << std::endl;
      break;
    case NC_BYTE:
      dataVector = new std::vector<int8_t>;

    case NC_NAT:
    default:
      ncDataType = NC_NAT;
      return;
    }
    ncDataType = type;
  }
#endif

  template <class DataType> std::vector<DataType> &getVector() {
    if (!dataVector) {
      std::cerr << "ERROR: Type for data not set. Will crash." << std::endl;
    }
    return *static_cast<std::vector<DataType> *>(dataVector);
  }

private:
  int ncDataType{0};
};

// TODO we should have the option to check if there is already a file on disk
// and start with that data.

class DiskBufferNetCDFData : public DiskBuffer<NetCDFData> {
public:
  DiskBufferNetCDFData(std::string id, std::string fileName = "",
                       std::string path = "", uint16_t size = 2)
      : DiskBuffer<NetCDFData>(id, fileName, path, size) {
#ifndef TINC_HAS_NETCDF
    std::cerr << "ERROR: DiskBufferNetCDFDouble built wihtout NetCDF support"
              << std::endl;
    assert(0 == 1);
#endif
  }

  /**
   * @brief Set name of variable in NetCDF file
   * @param name
   */
  void setDataFieldName(std::string name) { mDataFieldName = name; }

protected:
  virtual bool parseFile(std::string fileName,
                         std::shared_ptr<NetCDFData> newData) {
    bool ret = false;

    int ncid, retval;

#ifdef TINC_HAS_NETCDF
    if ((retval = nc_open((m_path + fileName).c_str(), NC_NOWRITE, &ncid))) {
      goto done;
    }
    int varid;
    if ((retval = nc_inq_varid(ncid, mDataFieldName.c_str(), &varid))) {
      goto done;
    }

    nc_type xtypep;
    char name[32];
    int ndimsp;
    int dimidsp[32];
    int nattsp;
    if ((retval = nc_inq_var(ncid, varid, name, &xtypep, &ndimsp, dimidsp,
                             &nattsp))) {
      goto done;
    }

    size_t lenp;
    if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &lenp))) {
      goto done;
    }
    newData->setType(xtypep);
    /* Read the data. */
    if (xtypep == NC_SHORT) {
      auto &data = newData->getVector<int16_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_short(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_INT) {
      auto &data = newData->getVector<int32_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_int(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_FLOAT) {
      auto &data = newData->getVector<float>();
      data.resize(lenp);
      if ((retval = nc_get_var_float(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_DOUBLE) {
      auto &data = newData->getVector<double>();
      data.resize(lenp);
      if ((retval = nc_get_var_double(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_UBYTE) {
      auto &data = newData->getVector<uint8_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_ubyte(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_USHORT) {
      auto &data = newData->getVector<uint16_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_ushort(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_UINT) {
      auto &data = newData->getVector<uint32_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_uint(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_INT64) {
      auto &data = newData->getVector<int64_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_longlong(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_UINT64) {
      auto &data = newData->getVector<uint64_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_ulonglong(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_STRING) {
      std::cerr << "string not yet supported in DisnkBufferNetCDF" << std::endl;
      //        auto &data = newData->get<uint64_t>();
      //        data.resize(lenp);
      //        if ((retval = nc_get_var_string(ncid, varid, data.data()))) {
      //            goto done;
      //        }
    } else if (xtypep == NC_CHAR) {
      auto &data = newData->getVector<uint8_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_ubyte(ncid, varid, data.data()))) {
        goto done;
      }
    } else if (xtypep == NC_BYTE) {
      auto &data = newData->getVector<int8_t>();
      data.resize(lenp);
      if ((retval = nc_get_var_schar(ncid, varid, data.data()))) {
        goto done;
      }
    } else {
      std::cerr << "Usupported NC type" << std::endl;
      return false;
    }

    // Read attributes
    for (int i = 0; i < nattsp; i++) {
      char name[128];
      nc_type xtypep;
      size_t lenp;
      if (!(retval = nc_inq_attname(ncid, varid, i, name))) {
        if ((retval = nc_inq_att(ncid, varid, name, &xtypep, &lenp))) {
          std::cerr << "ERROR reading netcdf attribute" << std::endl;
          continue;
        }
        if (lenp > 1) {
          std::cerr << "ERROR unsupported: attribute vectors. Use single values"
                    << std::endl;
        }
        auto &attr = newData->attributes;
        attr.clear();

        if (xtypep == NC_SHORT) {
          int16_t val;
          if ((retval = nc_get_att_short(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_INT) {
          int32_t val;
          if ((retval = nc_get_att_int(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_FLOAT) {
          float val;
          if ((retval = nc_get_att_float(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_DOUBLE) {
          double val;
          if ((retval = nc_get_att_double(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_UBYTE) {
          uint8_t val;
          if ((retval = nc_get_att_ubyte(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_USHORT) {
          uint16_t val;
          if ((retval = nc_get_att_ushort(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_UINT) {
          uint32_t val;
          if ((retval = nc_get_att_uint(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_INT64) {
          int64_t val;
          if ((retval = nc_get_att_longlong(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_UINT64) {
          uint64_t val;
          if ((retval = nc_get_att_ulonglong(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_STRING) {
          std::cerr << "string not yet supported in DiskBufferNetCDF"
                    << std::endl;
          continue;
          //        auto &data = newData->get<uint64_t>();
          //        data.resize(lenp);
          //        if ((retval = nc_get_var_string(ncid, varid, data.data())))
          //        {
          //            goto done;
          //        }
        } else if (xtypep == NC_CHAR) {
          uint8_t val;
          if ((retval = nc_get_att_ubyte(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else if (xtypep == NC_BYTE) {
          int8_t val;
          if ((retval = nc_get_att_schar(ncid, varid, name, &val))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
          newData->attributes[name] = VariantValue(val);
        } else {
          std::cerr << "Usupported NC type" << std::endl;
          return false;
        }
      }
    }

    ret = true;
  done:
    if ((retval = nc_close(ncid))) {
      std::cerr << "ERROR closing NetCDF file" << std::endl;
    }
#endif
    return ret;
  }

  bool encodeData(std::string fileName, NetCDFData &newData) override {
    int retval;
#ifdef TINC_HAS_NETCDF

    size_t len;
    void *data_ptr;
    nc_type xtype = newData.getType();

    if (xtype == NC_SHORT) {
      auto &data = newData.getVector<int16_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_INT) {
      auto &data = newData.getVector<int32_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_FLOAT) {
      auto &data = newData.getVector<float>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_DOUBLE) {
      auto &data = newData.getVector<double>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_UBYTE) {
      auto &data = newData.getVector<uint8_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_USHORT) {
      auto &data = newData.getVector<uint16_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_UINT) {
      auto &data = newData.getVector<uint32_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_INT64) {
      auto &data = newData.getVector<int64_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_UINT64) {
      auto &data = newData.getVector<uint64_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_STRING) {
      std::cerr << "string not yet supported in DiskBufferNetCDF" << std::endl;
      //        auto &data = newData->get<uint64_t>();
      //        data.resize(lenp);
      //        if ((retval = nc_get_var_string(ncid, varid, data.data()))) {
      //            goto done;
      //        }
    } else if (xtype == NC_CHAR) {
      auto &data = newData.getVector<int8_t>();
      data_ptr = data.data();
      len = data.size();
    } else if (xtype == NC_BYTE) {
      auto &data = newData.getVector<int8_t>();
      data_ptr = data.data();
      len = data.size();
    } else {
      std::cerr << "Usupported NC type" << std::endl;
      return false;
    }

    int *nattsp = nullptr;
    int ncid;
    int dimid;
    char name[32];
    int ndims = 1;
    int varidp;

    if ((retval = nc_create((m_path + fileName).c_str(), NC_CLOBBER, &ncid))) {
      goto done;
    }

    if ((retval = nc_def_dim(ncid, (mDataFieldName + "_dim").c_str(), len,
                             &dimid))) {
      goto done;
    }

    int dimids[1] = {dimid};
    if ((retval = nc_def_var(ncid, mDataFieldName.c_str(), xtype, ndims, dimids,
                             &varidp))) {
      goto done;
    }

    // attributes
    for (auto &attr : newData.attributes) {
      nc_type atttype = attr.second.type;

      if (atttype == NC_SHORT) {
        int16_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_INT) {
        int32_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_FLOAT) {
        float val = attr.second.valueDouble;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_DOUBLE) {
        double val = attr.second.valueDouble;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_UBYTE) {
        uint8_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_USHORT) {
        uint16_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_UINT) {
        uint32_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_INT64) {
        int64_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_UINT64) {
        uint64_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_STRING) {
        std::cerr << "string not yet supported in DiskBufferNetCDF"
                  << std::endl;
        //        auto &data = newData->get<uint64_t>();
        //        data.resize(lenp);
        //        if ((retval = nc_get_var_string(ncid, varid, data.data()))) {
        //            goto done;
        //        }
      } else if (atttype == NC_CHAR) {
        uint8_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else if (atttype == NC_BYTE) {
        int8_t val = attr.second.valueInt64;
        nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
      } else {
        std::cerr << "Usupported NC type" << std::endl;
        return false;
      }
    }

    if ((retval = nc_enddef(ncid))) {
      goto done;
    }

    if ((retval = nc_put_var(ncid, varidp, data_ptr))) {
      goto done;
    }

    if ((retval = nc_close(ncid))) {
      std::cerr << "ERROR closing NetCDF file" << std::endl;
      goto done;
    }
    return true;
  done:
#endif
    return false;
  }

  std::string mDataFieldName{"data"};
};

} // namespace tinc

#endif // DISKBUFFERNETCDF_HPP
