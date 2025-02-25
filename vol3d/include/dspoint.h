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

#ifndef DSPoint_H
#define DSPoint_H
#include <cmath>
#include <iostream>
#include <triple.h>
#include <ipoint3d.h>


class DSPoint {
public:
  DSPoint(float x=0,float y=0, float z=0) : x(x), y(y), z(z) {}
  DSPoint(IPoint3D &ip) : x(float(ip.x)), y(float(ip.y)), z(float(ip.z)) {}
  template<class T> DSPoint operator=(const Triple<T>& f) { return DSPoint(f.a,f.b,f.c); }
  template<class T> DSPoint(const Triple<T>& f) : x(f.a), y(f.b), z(f.c) {}
  DSPoint& operator+=(const DSPoint& v2) { x+=v2.x; y+=v2.y; z+=v2.z; return *this; }
  DSPoint& operator-=(const DSPoint& v2) { x-=v2.x; y-=v2.y; z-=v2.z; return *this; }
  DSPoint& operator*=(const float f) { x*=f; y*=f; z*=f; return *this; }
  DSPoint& operator*=(const int f) { x*=f; y*=f; z*=f; return *this; }
  DSPoint& operator*=(const DSPoint& f) { x*=f.x; y*=f.y; z*=f.z; return *this; }
  DSPoint& operator/=(const float f) { x/=f; y/=f; z/=f; return *this; }
  DSPoint& operator/=(const int f) { x/=f; y/=f; z/=f; return *this; }
  DSPoint& operator/=(const unsigned int f) { x/=f; y/=f; z/=f; return *this; }
  DSPoint operator-() const{ return DSPoint(-x,-y,-z); }
  inline float pwr() const { return x*x+y*y+z*z; }
  inline float mag() const { return sqrtf(x*x+y*y+z*z); }
  DSPoint abs() { return DSPoint(std::abs(x),std::abs(y),std::abs(z)); }
  DSPoint operator*(const float s) const { return DSPoint(x*s,y*s,z*s); }  
  DSPoint operator*(const DSPoint &s) const { return DSPoint(x*s.x,y*s.y,z*s.z); }
  DSPoint operator/(const float s) const { return DSPoint(x/s,y/s,z/s); }
  DSPoint unit() const { return DSPoint(*this)/=sqrtf(pwr()); }
  float dot(const DSPoint& v2) const { return x*v2.x + y*v2.y + z*v2.z; }
  DSPoint cross(const DSPoint& v2) const { return DSPoint(y*v2.z - z*v2.y, z*v2.x - x*v2.z, x*v2.y - y*v2.x); }
  void scale(const DSPoint& sv) { x *= sv.x; y *= sv.y; z *= sv.z; }
  float x,y,z;
};

inline DSPoint operator*(const float s, const DSPoint &p) { return p*s; }

inline DSPoint cross(const DSPoint& v1, const DSPoint& v2)
{
  return DSPoint(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

inline DSPoint operator+(const DSPoint& v1, const DSPoint& v2)
{
  return DSPoint(v1) += v2;
}

inline DSPoint operator-(const DSPoint& v1, const DSPoint& v2)
{
  return DSPoint(v1) -= v2;;
}

inline DSPoint operator/(const DSPoint& v1, const int f)
{
  return DSPoint(v1) /= f;
}

inline DSPoint abs(const DSPoint &p)
{
  return DSPoint(std::abs(p.x),std::abs(p.y),std::abs(p.z));
}

inline DSPoint operator*(const DSPoint& v1, const int f)
{
  return DSPoint(v1) *= f;
}

inline float dot(const DSPoint& v1, const DSPoint& v2)
{
  return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

inline float square(const float a) { return a*a; }

inline std::istream& operator>>(std::istream& s, DSPoint& p)
{
  return s>>p.x>>p.y>>p.z;
}

inline std::ostream& operator<<(std::ostream& s, const DSPoint& p)
{
  return s<<p.x<<' '<<p.y<<' '<<p.z;
}

#endif
