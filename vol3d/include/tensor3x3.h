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

#ifndef Tensor3x3_H
#define Tensor3x3_H

template <class T>
class Tensor3x3 {
public:
  Tensor3x3(const T xx=0, const T xy=0, const T xz=0,
            const T yx=0, const T yy=0, const T yz=0,
            const T zx=0, const T zy=0, const T zz=0)
  {
    d[0] = xx; d[1] = xy; d[2] = xz;
    d[3] = yx; d[4] = yy; d[5] = yz;
    d[6] = zx; d[7] = zy; d[8] = zz;
  }
  Tensor3x3(const int /*n*/) { } // assignment operator required by Vol3D
  T* operator[](const int n) { return d+n*3; }
  T &operator()(const int n) { return d[n]; }
  T  operator()(const int n) const { return d[n]; }
  T d[9];
};

typedef Tensor3x3<float>  Tensor3x3f;
typedef Tensor3x3<double> Tensor3x3d;

#endif
