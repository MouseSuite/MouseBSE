// Copyright (C) 2025 The Regents of the University of California
//
// Created by David W. Shattuck, Ph.D.
//
// This file is part of MouseBSE.
//
// MouseBSE is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, version 2.1.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//

#ifndef SILT_ZStream_H
#define SILT_ZStream_H

#include <sstream>

#ifdef WIN32
#ifndef ZLIB_WINAPI
#define ZLIB_WINAPI
#endif
#endif

#include <zlib.h>

namespace SILT {
//! \brief A simple input stream class for interfacing with gzipped files.
//! \details This class is limited compared to other stream classes and only implements basic read and seek functions.
//! \author david w. shattuck
//! \date 30 April 2010
class izstream {
public:
  izstream() {}
  izstream(std::string ifname) { open(ifname); }
  ~izstream() { close(); }
  z_off_t seekg(z_off_t offset, int whence=SEEK_SET) { return  gzseek (fp, offset, whence); }
  bool open(const char *ifname)
  {
    if (fp) close();
    fp = ::gzopen(ifname, "rb");
    return (fp!=nullptr);
  }
  bool open(const std::string ifname)
  {
    return open(ifname.c_str());
  }
  bool operator!() { return (fp==nullptr); }
  bool close()
  {
    int id = gzclose(fp);
    fp=nullptr;
    return (id!=0);
  }
  auto read(void* buf, unsigned len)
  {
    return ::gzread(fp, buf, len);
  }
private:
  gzFile fp{nullptr};
};

//! \brief A simple output stream class for interfacing with gzipped files.
//! \details This class is limited compared to other stream classes and only implements basic write functionality.
//! \author david w. shattuck
//! \date 30 April 2010
class ozstream {
public:
  ozstream() {}
  ozstream(std::string ofname, int level = Z_DEFAULT_COMPRESSION) { open(ofname,level); }
  ~ozstream() { close(); }
  bool open(const char *ofname, int level = Z_DEFAULT_COMPRESSION)
  {
    if (fp) close();
    std::ostringstream mode;
    mode<<"wb";
    if (level>=0) mode<<level;
    fp = ::gzopen(ofname, mode.str().c_str());
    return (fp!=nullptr);
  }
  bool open(const std::string ifname, int level = Z_DEFAULT_COMPRESSION)
  {
    return open(ifname.c_str(), level);
  }
  bool close()
  {
    int id = gzclose(fp);
    fp=nullptr;
    return (id!=0);
  }
  bool operator!() { return (fp==nullptr); }
  auto write(void* buf, unsigned len)
  {
    return ::gzwrite(fp, buf, len);
  }
private:
  gzFile fp{nullptr};
};

} // end of namespace SILT

#endif
