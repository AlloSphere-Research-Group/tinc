#include "tinc/DiskBufferNetCDFData.hpp"

using namespace tinc;

NetCDFData::~NetCDFData() { deleteData(); }

int NetCDFData::getType() { return ncDataType; }

void NetCDFData::setType(int type) {
  if (type == ncDataType) {
    return;
  }
  deleteData();
  switch (type) {
  case al::VariantType::VARIANT_INT16:
    dataVector = new std::vector<int16_t>;
    break;
  case al::VariantType::VARIANT_INT32:
    dataVector = new std::vector<int32_t>;
    break;
  case al::VariantType::VARIANT_FLOAT:
    dataVector = new std::vector<float>;
    break;
  case al::VariantType::VARIANT_DOUBLE:
    dataVector = new std::vector<double>;
    break;
  case al::VariantType::VARIANT_UINT8:
    dataVector = new std::vector<uint8_t>;
    break;
  case al::VariantType::VARIANT_CHAR:
    dataVector = new std::vector<char>;
    break;
  case al::VariantType::VARIANT_UINT16:
    dataVector = new std::vector<uint16_t>;
    break;
  case al::VariantType::VARIANT_UINT32:
    dataVector = new std::vector<int32_t>;
    break;
  case al::VariantType::VARIANT_INT64:
    dataVector = new std::vector<int64_t>;
    break;
  case al::VariantType::VARIANT_UINT64:
    dataVector = new std::vector<uint64_t>;
    break;
  case al::VariantType::VARIANT_STRING:
    dataVector = new std::vector<std::string>;

    std::cerr << "string not yet supported in DisnkBufferNetCDF" << std::endl;
    break;
  case al::VariantType::VARIANT_INT8:
    dataVector = new std::vector<int8_t>;

  case al::VariantType::VARIANT_NONE:
  default:
    ncDataType = al::VariantType::VARIANT_NONE;
    return;
  }

  ncDataType = (al::VariantType)type;
}

void NetCDFData::deleteData() {
  if (dataVector) {
    switch (ncDataType) {
    case SHORT:
      delete static_cast<std::vector<int16_t> *>(dataVector);
      break;
    case INT:
      delete static_cast<std::vector<int32_t> *>(dataVector);
      break;
    case FLOAT:
      delete static_cast<std::vector<float> *>(dataVector);
      break;
    case DOUBLE:
      delete static_cast<std::vector<double> *>(dataVector);
      break;
    case UBYTE:
    case CHAR:
      delete static_cast<std::vector<uint8_t> *>(dataVector);
      break;
    case USHORT:
      delete static_cast<std::vector<uint16_t> *>(dataVector);
      break;
    case UINT:
      delete static_cast<std::vector<int32_t> *>(dataVector);
      break;
    case INT64:
      delete static_cast<std::vector<int64_t> *>(dataVector);
      break;
    case UINT64:
      delete static_cast<std::vector<uint64_t> *>(dataVector);
      break;
    case STRING:
      delete static_cast<std::vector<std::string> *>(dataVector);
      std::cerr << "string not yet supported in DisnkBufferNetCDF" << std::endl;
      break;
    case BYTE:
      delete static_cast<std::vector<int8_t> *>(dataVector);
    }

    dataVector = nullptr;
  }
}

bool DiskBufferNetCDFData::parseFile(std::string fileName,
                                     std::shared_ptr<NetCDFData> newData) {
  bool ret = false;

  int ncid, retval;

#ifdef TINC_HAS_NETCDF
  if ((retval =
           nc_open((getFullPath() + fileName).c_str(), NC_NOWRITE, &ncid))) {
    std::cout << __FILE__ << ":" << __LINE__
              << " ERROR opening netcdf file:" << getFullPath() + fileName
              << std::endl;
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
  if ((retval =
           nc_inq_var(ncid, varid, name, &xtypep, &ndimsp, dimidsp, &nattsp))) {
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
#ifdef AL_WINDOWS
    if ((retval = nc_get_var_longlong(ncid, varid, data.data()))) {
      goto done;
    }
#else
    if ((retval = nc_get_var(ncid, varid, data.data()))) {
      goto done;
    }
#endif
  } else if (xtypep == NC_UINT64) {
    auto &data = newData->getVector<uint64_t>();
    data.resize(lenp);
#ifdef AL_WINDOWS
    if ((retval = nc_get_var_ulonglong(ncid, varid, data.data()))) {
      goto done;
    }
#else
    if ((retval = nc_get_var(ncid, varid, data.data()))) {
      goto done;
    }
#endif
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
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_INT) {
        int32_t val;
        if ((retval = nc_get_att_int(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_FLOAT) {
        float val;
        if ((retval = nc_get_att_float(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_DOUBLE) {
        double val;
        if ((retval = nc_get_att_double(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_UBYTE) {
        uint8_t val;
        if ((retval = nc_get_att_ubyte(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_USHORT) {
        uint16_t val;
        if ((retval = nc_get_att_ushort(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_UINT) {
        uint32_t val;
        if ((retval = nc_get_att_uint(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_INT64) {
        int64_t val;
        if (std::is_same<int64_t, long long>::value) {
          if ((retval = nc_get_att_longlong(
                   ncid, varid, name, reinterpret_cast<long long *>(&val)))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
        } else if (std::is_same<int64_t, long>::value) {
          if ((retval = nc_get_att_long(ncid, varid, name,
                                        reinterpret_cast<long *>(&val)))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
        } else {
          std::cerr << "int64_t not supported properly" << std::endl;
          return false;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_UINT64) {
        uint64_t val;
        if (std::is_same<uint64_t, unsigned long long>::value) {
          if ((retval = nc_get_att_ulonglong(
                   ncid, varid, name,
                   reinterpret_cast<unsigned long long *>(&val)))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
        } else if (std::is_same<uint64_t, unsigned long>::value) {
          if ((retval = nc_get_att(ncid, varid, name,
                                   reinterpret_cast<unsigned long *>(&val)))) {
            std::cerr << "ERROR getting attribute value" << std::endl;
            continue;
          }
        } else {
          std::cerr << "uint64_t not supported properly" << std::endl;
          return false;
        }
        newData->attributes[name] = al::VariantValue(val);
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
        newData->attributes[name] = al::VariantValue(val);
      } else if (xtypep == NC_BYTE) {
        int8_t val;
        if ((retval = nc_get_att_schar(ncid, varid, name, &val))) {
          std::cerr << "ERROR getting attribute value" << std::endl;
          continue;
        }
        newData->attributes[name] = al::VariantValue(val);
      } else {
        std::cerr << "Unsupported NC type" << std::endl;
        return false;
      }
    }
  }

  ret = true;
  if ((retval = nc_close(ncid))) {
    std::cerr << __FILE__ << ":" << __LINE__ << "ERROR closing NetCDF file "
              << getFullPath() + fileName << std::endl;
  }
done:
#endif
  return ret;
}

bool DiskBufferNetCDFData::encodeData(std::string fileName,
                                      NetCDFData &newData) {
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
  //    char name[32];
  int ndims = 1;
  int varidp;
  nc_type atttype;
  int dimids[1];

  if ((retval = nc_create((getFullPath() + fileName).c_str(),
                          NC_CLOBBER | NC_NETCDF4, &ncid))) {
    goto done;
  }

  if ((retval =
           nc_def_dim(ncid, (mDataFieldName + "_dim").c_str(), len, &dimid))) {
    goto done;
  }

  dimids[0] = dimid;
  if ((retval = nc_def_var(ncid, mDataFieldName.c_str(), xtype, ndims, dimids,
                           &varidp))) {
    goto done;
  }

  // attributes
  for (auto &attr : newData.attributes) {
    atttype = attr.second.type();

    if (atttype == NC_SHORT) {
      int16_t val = attr.second.get<int16_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_INT) {
      int32_t val = attr.second.get<int32_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_FLOAT) {
      float val = attr.second.get<float>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_DOUBLE) {
      double val = attr.second.get<double>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_UBYTE) {
      uint8_t val = attr.second.get<uint8_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_USHORT) {
      uint16_t val = attr.second.get<uint16_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_UINT) {
      uint32_t val = attr.second.get<uint32_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_INT64) {
      int64_t val = attr.second.get<int64_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_UINT64) {
      uint64_t val = attr.second.get<uint64_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_STRING) {
      std::cerr << "string not yet supported in DiskBufferNetCDF" << std::endl;
      //        auto &data = newData->get<uint64_t>();
      //        data.resize(lenp);
      //        if ((retval = nc_get_var_string(ncid, varid, data.data()))) {
      //            goto done;
      //        }
    } else if (atttype == NC_CHAR) {
      uint8_t val = attr.second.get<uint8_t>();
      nc_put_att(ncid, varidp, attr.first.c_str(), atttype, 1, &val);
    } else if (atttype == NC_BYTE) {
      int8_t val = attr.second.get<int8_t>();
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
