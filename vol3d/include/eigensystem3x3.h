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

#ifndef EigenSystem3x3_H
#define EigenSystem3x3_H

#include <dspoint.h>

class EigenSystem3x3f {
public:
  EigenSystem3x3f(const float l0_=0, const float l1_=0, const float l2_=0) : l0(l0_), l1(l1_), l2(l2_) {}
  DSPoint rotate(const DSPoint &d) const
  {
    return DSPoint(
      v0.x * d.x + v1.x * d.y + v2.x * d.z,
      v0.y * d.x + v1.y * d.y + v2.y * d.z,
      v0.z * d.x + v1.z * d.y + v2.z * d.z
    );
  }
  DSPoint v0,v1,v2;
  float l0,l1,l2;
};

#endif
