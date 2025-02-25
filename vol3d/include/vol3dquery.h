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

#ifndef Vol3DQuery_H
#define Vol3DQuery_H

#include <strutil.h>
#include <DS/getfileinfo.h>
#include <DS/dsheader.h>
#include <iostream>
#include <zstream.h>
#include <tensor3x3.h>
#include <eigensystem3x3.h>
#include <vol3dbase.h>
#include <niftiinfo.h>

class Vol3DQuery {
public:
  Vol3DQuery();
  bool parseAnalyze(std::string fname);
  bool parseNIFTI(std::string fname);
  bool parseNIFTIheader(nifti_1_header hdr);
  static void swapNIFTIHeader(nifti_1_header &hdr);
  bool findFile(std::string &queryname);
  bool query(std::string ifname);
  std::string description;
  std::string filename;
  std::string headerFilename;
  HeaderType headerType;
  SILT::DataType datatype;
  int cx,cy,cz;
  float rx,ry,rz;
  DSPoint origin;
  DSPoint e1,e2,e3; // eigen vectors
  float sx,sy,sz; // scale factors
  std::streamsize filesize;
  int datastart;
  std::streamsize sizeOnDisk;
  bool swapped;
  bool compressed;
  int bitsPerVoxel;
  SILT::NIFTIInfo niftiInfo;
};

#endif
