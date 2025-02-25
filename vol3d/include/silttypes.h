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

#ifndef SiltTypes_H
#define SiltTypes_H

#include <string>

enum class HeaderType { // TODO: add enum classes here
  NoHeader = 0,
  Analyze = 1,
  NIFTI=2,
  NIFTI_TWO_FILE=3,
  DICOM = 4,
};

namespace SILT {
enum DataType {
  Unknown = -1,
  Uint8 = 0, Sint8 = 1,
  Uint16 = 2, Sint16 = 3,
  Uint32 = 4, Sint32 = 5,
  Uint64 = 6, Sint64 = 7,
  Float32 = 8, Float64 = 9,
  RGB8 = 10, RGBA8 = 11,
  Tensor3x3F = 20,
  Eigensystem3x3f = 21,
  Vector3F = 22,
};
inline std::string datatypeName(const DataType typecode)
{
  switch (typecode)
  {
    case SILT::Uint8 : return "uint8"; break;
    case SILT::Sint8 : return "sint8"; break;
    case SILT::Uint16 : return "uint16"; break;
    case SILT::Sint16 : return "sint16"; break;
    case SILT::Uint32 : return "uint32"; break;
    case SILT::Sint32 : return "sint32"; break;
    case SILT::Float32 : return "float32"; break;
    case SILT::Float64 : return "float64"; break;
    case SILT::RGB8    : return "rgb8"; break;
    case SILT::RGBA8   : return "rgba8"; break;
    case SILT::Tensor3x3F : return "tensor3x3f"; break;
    case SILT::Eigensystem3x3f : return "eigensystem3x3f"; break;
    case SILT::Vector3F : return "vector3f"; break;
    case SILT::Unknown :
    default: return "unknown"; break;
  }
  return "unknown";
}
};

#endif
