// Copyright (C) 2025 The Regents of the University of California and
// the University of Southern California
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

#ifndef Morph32_H
#define Morph32_H

#include <vol3d.h>
#include <vbit.h>

class Morph32 {
public:
  void load(std::vector<uint32> &a, Vol3D<VBit> &v)
  {
    const auto ds = v.size();
    for (size_t i=0;i<ds;i++) a[i] = v[i].data;
  }
  Morph32() : cx(0), cy(0), cz(0), slicesize(0)
  {
  }
  ~Morph32() {}
  void releaseMemory();
  void init(int cx_, int cy_, int cz_);
  void setup(Vol3D<VBit> &v)
  {
    if ((v.cx!=cx)||(v.cy!=cy)||(v.cz!=cz))
      init(v.cx,v.cy,v.cz);
  }
  bool erodeR(Vol3D<VBit> &v)
  {
    setup(v);
    load(volA,v);
    return erodeR(&volA[0],v.raw32());
  }
  bool dilateR(Vol3D<VBit> &v)
  {
    setup(v);
    load(volA,v);
    return dilateR(&volA[0],v.raw32());
  }
  bool erodeC(Vol3D<VBit> &v)
  {
    setup(v);
    load(volA,v);
    return erodeC(&volA[0],v.raw32());
  }
  bool dilateC(Vol3D<VBit> &v)
  {
    setup(v);
    load(volA,v);
    return dilateC(&volA[0],v.raw32());
  }
  bool erodeO2(Vol3D<VBit> &v)
  {
    setup(v);
    return erodeO2(v.raw32());
  }
  bool dilateO2(Vol3D<VBit> &v)
  {
    setup(v);
    return dilateO2(v.raw32());
  }
  bool dilateO2(uint32 *a) { return dilateO2(a,a); }
  bool erodeO2(uint32 *a) { return erodeO2(a,a); }
  bool dilateO2(uint32 *a, uint32 *b);
  bool erodeO2(uint32 *a, uint32 *b);
  bool dilateC(uint32 *a, uint32 *b);
  bool erodeC (uint32 *a, uint32 *b);
  bool dilateR(uint32 *a, uint32 *b);
  bool erodeR (uint32 *a, uint32 *b);
  void dilateX32(uint32 *in, uint32 *out, const int cx, const int n);
  void dilateY32(uint32 *in, uint32 *out, const int cx, const int cy, const int cz);
  void erodeY32(uint32 *in, uint32 *out, const int cx, const int cy, const int cz);
  void erodeX32(uint32 *in, uint32 *out, const int cx, const int n);
  void dilateY32(uint32 *in, uint32 *inB, uint32 *out, const int cx, const int cy, const int cz);
  void erodeY32(uint32 *in, uint32 *inB, uint32 *out, const int cx, const int cy, const int cz);
protected:
  uint32 cx,cy,cz;
  size_t slicesize;
  std::vector<uint32> sliceA,volA,volB;
};

#endif
