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

#ifndef NIFTIParser_H
#define NIFTIParser_H

#include <niftiinfo.h>
#include <silttypes.h>
#include <string>
#include <dspoint.h>

class NIFTIParser {
public:
  void swapNIFTIHeader(nifti_1_header &hdr);
  bool parseNIFTI(std::string ifname);
  std::string filename;
  std::string headerFilename;
  std::streamsize sizeOnDisk;
  int datastart;
  HeaderType headerType;
  SILT::DataType datatype;
  bool swapped;
  std::string description;
  int cx,cy,cz,ct,cv;
  float rx,ry,rz;
  DSPoint e1,e2,e3; // eigen vectors
  DSPoint origin;
  int bitsPerVoxel;
  SILT::NIFTIInfo niftiInfo;
};

#endif
