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

#ifndef Point3D_H
#define Point3D_H

template <class T> class Point3D
{
public:
  Point3D(const T &x=0, const T &y=0, const T&z=0) : x( x), y(y), z(z)  {}
  template <class S>
  Point3D(const Point3D<S> & v2) : x(v2.x), y(v2.y), z(v2.z)  {}
  Point3D<T>& operator+=(const Point3D<T>& v2) { x+=v2.x; y+=v2.y; z+=v2.z; return *this; }
  Point3D<T>& operator-=(const Point3D<T>& v2) { x-=v2.x; y-=v2.y; z-=v2.z; return *this; }
  template <class S> Point3D<T>& operator/=(const S &s) { x/=s; y/=s; z/=s; return *this; }
  T x,y,z;
};

template <class T>
inline Point3D<T> operator+(const Point3D<T> & v1, const Point3D<T> & v2)
{
  return Point3D<T> (v1) += v2;
}

template <class T>
inline Point3D<T>  operator-(const Point3D<T> & v1, const Point3D<T> & v2)
{
  return Point3D<T>(v1) -= v2;;
}

template <class T, class S>
inline Point3D<T>  operator/(const Point3D<T> & v1, const S & s)
{
  return Point3D<T>(v1)/=s;;
}

typedef Point3D<int> IPoint3D;

#endif
