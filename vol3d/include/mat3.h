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

#ifndef SILT_Mat3_H
#define SILT_Mat3_H

#include <dspoint.h>
#include <ipoint3d.h>

namespace SILT {

template <class ArrayT>
class Mat3 {
public:
  ArrayT
  m00,m01,m02,
  m10,m11,m12,
  m20,m21,m22;
  enum DefaultMatrix { Blank=0, Identity=1 };
  Mat3(ArrayT m00=0, ArrayT m01=0, ArrayT m02=0,
       ArrayT m10=0, ArrayT m11=0, ArrayT m12=0,
       ArrayT m20=0, ArrayT m21=0, ArrayT m22=0)
       : m00(m00), m01(m01), m02(m02),
         m10(m10), m11(m11), m12(m12),
         m20(m20), m21(m21), m22(m22)
  {
  }
  Mat3(DefaultMatrix defaultMatrixType)	:
    m00(0), m01(0), m02(0),
    m10(0), m11(0), m12(0),
    m20(0), m21(0), m22(0)
  {
    switch (defaultMatrixType) {
      case Identity : m00 = m11 = m22 = 1;
      case Blank :
      default:
        break;
    }
  }
  template <class T>
  Mat3<T> convert()
  {
    return Mat3<T>((T)m00,(T)m01,(T)m02,(T)m10,(T)m11,(T)m12,(T)m20,(T)m21,(T)m22);
  }
  static Mat3 fromRows(const DSPoint &row0, const DSPoint &row1, const DSPoint &row2)
  {
    return Mat3(row0.x,row0.y,row0.z,
                row1.x,row1.y,row1.z,
                row2.x,row2.y,row2.z);
  }
  static Mat3 fromColumns(const DSPoint &col0, const DSPoint &col1, const DSPoint &col2)
  {
    return Mat3(col0.x,col1.x,col2.x,
                col0.y,col1.y,col2.y,
                col0.z,col1.z,col2.z);
  }
  static SILT::Mat3<ArrayT> diagonal(const DSPoint &p)
  {
    return SILT::Mat3<ArrayT>(
          p.x,0,0,
          0,p.y,0,
          0,0,p.z);
  }
  double determinant()
  {
    return m00*(m11*m22-m12*m21)
          +m01*(m12*m20-m10*m22)
          +m02*(m10*m21-m11*m20);
  }
  DSPoint row0() const { return DSPoint(m00,m01,m02); }
  DSPoint row1() const { return DSPoint(m10,m11,m12); }
  DSPoint row2() const { return DSPoint(m20,m21,m22); }
  DSPoint col0() const { return DSPoint(m00,m10,m20); }
  DSPoint col1() const { return DSPoint(m01,m11,m21); }
  DSPoint col2() const { return DSPoint(m02,m12,m22); }
  DSPoint operator*(const DSPoint p) const
  {
    return DSPoint(m00*p.x+m01*p.y+m02*p.z,
                   m10*p.x+m11*p.y+m12*p.z,
                   m20*p.x+m21*p.y+m22*p.z);
  }
  Mat3& operator+=(const Mat3& m)
  {
    m00+=m.m00; m01+=m.m01; m02+=m.m02;
    m10+=m.m10; m11+=m.m11; m12+=m.m12;
    m20+=m.m20; m21+=m.m21; m22+=m.m22;
    return *this;
  }
  Mat3& operator-=(const Mat3& m)
  {
    m00-=m.m00; m01-=m.m01; m02-=m.m02;
    m10-=m.m10; m11-=m.m11; m12-=m.m12;
    m20-=m.m20; m21-=m.m21; m22-=m.m22;
    return *this;
  }
  inline Mat3 operator+(const Mat3& m) const
  {
    return Mat3(*this) += m;
  }
  inline Mat3 operator-(const Mat3& m) const
  {
    return Mat3(*this) -= m;
  }
  template <class T> Mat3& operator*=(const T &scalar) // scalar values
  {
    m00*=scalar; m01*=scalar; m02*=scalar;
    m10*=scalar; m11*=scalar; m12*=scalar;
    m20*=scalar; m21*=scalar; m22*=scalar;
    return *this;
  }
  template <class T> Mat3& operator/=(const T &scalar) // scalar values
  {
    m00/=scalar; m01/=scalar; m02/=scalar;
    m10/=scalar; m11/=scalar; m12/=scalar;
    m20/=scalar; m21/=scalar; m22/=scalar;
    return *this;
  }
  Mat3 transpose() const
  {
    return Mat3(m00,m10,m20,
                m01,m11,m21,
                m02,m12,m22);
  }
  Mat3 operator*(const Mat3 &m) const
  {
    return Mat3(
          m00*m.m00 + m01*m.m10 + m02*m.m20,
          m00*m.m01 + m01*m.m11 + m02*m.m21,
          m00*m.m02 + m01*m.m12 + m02*m.m22,

          m10*m.m00 + m11*m.m10 + m12*m.m20,
          m10*m.m01 + m11*m.m11 + m12*m.m21,
          m10*m.m02 + m11*m.m12 + m12*m.m22,

          m20*m.m00 + m21*m.m10 + m22*m.m20,
          m20*m.m01 + m21*m.m11 + m22*m.m21,
          m20*m.m02 + m21*m.m12 + m22*m.m22
          );
  }
};

template <class ArrayT>
inline std::ostream& operator<<(std::ostream& s, const Mat3<ArrayT>& m)
{
  return s<<m.m00<<' '<<m.m01<<' '<<m.m02<<'\n'
          <<m.m10<<' '<<m.m11<<' '<<m.m12<<'\n'
          <<m.m20<<' '<<m.m21<<' '<<m.m22<<'\n';
}

}
#endif

