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

#ifndef GetFileSize_H
#define GetFileSize_H

#include <iostream>
#include <filesystem>
#include <fstream>

inline auto fileExists(const std::string &filename)
{
  std::error_code ec;
  auto value=static_cast<std::streamsize>(std::filesystem::exists(filename,ec));
  if (ec)
  {
    std::cerr << "\nfile system error for "<<filename<<": "<< ec.message() <<'('<<ec.value()<<')'<<'\n';
  }
  return value;
}


inline std::streamsize getFileSize(const std::string &filename)
{
  if (!fileExists(filename))
  {
    return -1;
  }
  std::error_code ec;
  auto value=static_cast<std::streamsize>(std::filesystem::file_size(filename,ec));
  if (ec)
  {
    value=-1;
    std::cerr << "\nerror reading filesize for "<<filename<<": "<< ec.message() <<'('<<ec.value()<<')'<<'\n';
  }
  return value;
}

inline auto isFile(const std::string &filename)
{
  std::error_code ec;
  if (std::filesystem::is_regular_file(std::filesystem::status(filename,ec))) return true;
  if (ec.value()) std::cerr<<"error checking "<<filename<<" : "<<ec.message()<<'\t'<<ec.value()<<std::endl;
  return false;
}

inline std::streamsize getGZipFilesize(std::string s)
// warning: limited to 4GB files!
{
  std::ifstream ifile(s.c_str(),std::ios::binary);
  unsigned char buf[4];
  ifile.seekg( -4,std::ios::end);
  ifile.read(reinterpret_cast<char*>(buf),4);
  return ((buf[1]<<8) + buf[0]) + (((buf[3]<<8) + buf[2])<<16);
}

#endif
