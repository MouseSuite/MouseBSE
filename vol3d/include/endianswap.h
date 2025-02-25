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

#ifndef Endian_Swap_H
#define Endian_Swap_H

// adapted from http://www.codeguru.com/forum/showthread.php?s=&threadid=292902
#include <vol3ddatatypes.h>

namespace SILT {

inline bool isLittleEndian()
{
  unsigned int test = 0x000000FF;
  return (((char *)(&test))[0])!=0;
}

inline void endian_swap(unsigned short& x)
{
  x = (x>>8) |
      (x<<8);
}

inline void endian_swap(unsigned int& x)
{
  x = (x>>24) |
      ((x<<8) & 0x00FF0000) |
      ((x>>8) & 0x0000FF00) |
      (x<<24);
}

inline void endian_swap(uint64& x)
{
  x = (x>>56) |
      ((x<<40) & 0x00FF000000000000LL) |
      ((x<<24) & 0x0000FF0000000000LL) |
      ((x<<8)  & 0x000000FF00000000LL) |
      ((x>>8)  & 0x00000000FF000000LL) |
      ((x>>24) & 0x0000000000FF0000LL) |
      ((x>>40) & 0x000000000000FF00LL) |
      (x<<56);
}

inline void endian_swap(signed short& x) { return endian_swap(*reinterpret_cast<unsigned short *>(&x)); }
inline void endian_swap(signed int& x) { return endian_swap(*reinterpret_cast<unsigned int *>(&x)); }
inline void endian_swap(float& x) { return endian_swap(*reinterpret_cast<unsigned int *>(&x)); }
inline void endian_swap(double& x) { return endian_swap(*reinterpret_cast<uint64 *>(&x)); }
};

#endif
