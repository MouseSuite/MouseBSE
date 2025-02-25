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

#ifndef Vol3DUtils_H
#define Vol3DUtils_H
#undef max
#include <algorithm>
#include <vol3d.h>

inline bool opOr(Vol3D<uint8> &dst, Vol3D<uint8> &src)
{
  if (dst.isCompatible(src))
  {
    size_t ds = dst.size();
    uint8 *d = dst.start();
    uint8 *s = src.start();
    for (size_t i=0;i<ds;i++) d[i] |= s[i];
    return true;
  }
  else
  {
    const auto mx = std::max(dst.cx, src.cx);
    const auto my = std::max(dst.cy, src.cy);
    const auto mz = std::max(dst.cz, src.cz);
    for (decltype(dst.cz) z=0;z<mz;z++)
      for (decltype(dst.cy) y=0;y<my;y++)
        for (decltype(dst.cx) x=0;x<mx;x++)
          dst(x,y,z) |= src(x,y,z);
  }
  return false;
}

template <class T>
inline bool mask(Vol3D<T> &vImage, Vol3D<uint8> &vMask)
{
  if (!vImage.isCompatible(vMask)) { return false; }
  T *d = vImage.start();
  const uint8 *m = vMask.start();
  const auto ds = vImage.size();
  for (size_t i=0;i<ds;i++) if (!m[i]) d[i] = 0;
  return true;
}

#endif
