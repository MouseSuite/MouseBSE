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

#ifndef IPoint3D_H
#define IPoint3D_H

#include <point3d.h>
#include <iostream>

inline std::istream& operator>>(std::istream& s, IPoint3D& p)
{
  return s>>p.x>>p.y>>p.z;
}

inline std::ostream& operator<<(std::ostream& s, const IPoint3D& p)
{
  return s<<p.x<<' '<<p.y<<' '<<p.z;
}

#endif
