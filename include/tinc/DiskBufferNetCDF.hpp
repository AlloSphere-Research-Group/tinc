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

#ifdef TINC_HAS_NETCDF
#include <netcdf.h>
#endif

namespace tinc {

struct NC_Dimension {
  int dataType;
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
};

// TODO we should have the option to check if there is already a file on disk
// and start with that data.

class DiskBufferNetCDFDouble : public DiskBuffer<std::vector<double>> {
public:
  DiskBufferNetCDFDouble(std::string id, std::string fileName = "",
                         std::string path = "", uint16_t size = 2)
      : DiskBuffer<std::vector<double>>(id, fileName, path, size) {
#ifndef TINC_HAS_NETCDF
    std::cerr << "ERROR: DiskBufferNetCDFDouble built wihtout NetCDF support"
              << std::endl;
    assert(0 == 1);
#endif
  }

  bool updateData(std::string filename) {
    std::unique_lock<std::mutex> lk(mWriteLock);
    if (filename.size() > 0) {
      m_fileName = filename;
    }
    bool ret = false;
    auto buffer = getWritable();

    int ncid, retval;

#ifdef TINC_HAS_NETCDF
    int *nattsp = nullptr;
    /* Open the file. NC_NOWRITE tells netCDF we want read-only access
     * to the file.*/
    if ((retval = nc_open(filename.c_str(), NC_NOWRITE, &ncid))) {
      goto done;
    }
    int varid;
    if ((retval = nc_inq_varid(ncid, "data", &varid))) {
      goto done;
    }

    nc_type xtypep;
    char name[32];
    int ndimsp;
    int dimidsp[32];
    if ((retval = nc_inq_var(ncid, varid, name, &xtypep, &ndimsp, dimidsp,
                             nattsp))) {
      goto done;
    }

    size_t lenp;
    if ((retval = nc_inq_dimlen(ncid, dimidsp[0], &lenp))) {
      goto done;
    }
    buffer->resize(lenp);

    /* Read the data. */
    if ((retval = nc_get_var_double(ncid, varid, buffer->data()))) {
      goto done;
    }

    //    /* Check the data. */
    //    for (x = 0; x < NX; x++)
    //      for (y = 0; y < NY; y++)
    //        if (data_in[x][y] != x * NY + y)
    //          return ERRCODE;

    /* Close the file, freeing all resources. */
    if ((retval = nc_close(ncid))) {
      goto done;
    }
    ret = true;
    BufferManager<std::vector<double>>::doneWriting(buffer);
#endif
  done:
    for (auto cb : mUpdateCallbacks) {
      cb(ret);
    }
    return ret;
  }

protected:
  virtual bool parseFile(std::ifstream &file,
                         std::shared_ptr<std::vector<double>> newData) {

    return true;
  }
};

} // namespace tinc

#endif // DISKBUFFERNETCDF_HPP
