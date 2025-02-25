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

#ifndef Vol3DReorder_H
#define Vol3DReorder_H
// Author: David Shattuck
//
// This class encapsulates functions for reordering the data in a Vol3D object.
// Primarily, this is intended for ordering the data such that it is stored
// in RAS order, e.g., the fastest changing dimension goes from Left -> Right,
// then Posterior -> Anterior, then Inferior -> Superior
// In performing this reordering, the code will also update the orientation matrix,
// position vector, voxel dimensions, and voxel resolutions.
//
// The reordering could be improved by writing custom functions for the 6 possible
// permutations, and we can ignore the trivial case of Pijk, where the transform is
// identity. e.g., use
//   enum Permutation { Pijk=0, Pikj=1, Pjik=2, Pjki=3, Pkij=4, Pkji=5 };
// to identify the closest orientation of the volume, then identify the transform
// to bring it to Pijk.
//
// It would also be useful to be able to restore the order to the original file
// convention as it was loaded from disk.
//
// Some operations could be made faster by using pointer increments rather than
// the Vol3D position operator, but the current implementation is easier to read.

#include <string>
#include <sstream>
#include <dspoint.h>
#include <vol3d.h>

class Vol3DReorder {
public:
  static DSPoint codeToRASVector(char code);
  static std::string getOrientationRAS(const DSPoint &vector);
  template <class T>
  static void flipX(Vol3D<T> &volume)
  {
    const int cx=volume.cx;
    const int cy=volume.cy;
    const int cz=volume.cz;
    const int cx_2=cx/2;
    for (int z=0;z<cz;z++)
      for (int y=0;y<cy;y++)
        for (int x=0;x<cx_2;x++)
        {
          std::swap(volume(x,y,z),volume(cx-1-x,y,z));
        }
  }
  template <class T>
  static void flipY(Vol3D<T> &volume)
  {
    const int cx=volume.cx;
    const int cy=volume.cy;
    const int cz=volume.cz;
    const int cy_2=cy/2;
    for (int z=0;z<cz;z++)
      for (int y=0;y<cy_2;y++)
        for (int x=0;x<cx;x++)
          std::swap(volume(x,y,z),volume(x,cy-1-y,z));
  }
  template <class T>
  static void flipZ(Vol3D<T> &volume)
  {
    const int cx=volume.cx;
    const int cy=volume.cy;
    const int cz=volume.cz;
    const int cz_2=cz/2;
    for (int z=0;z<cz_2;z++)
      for (int y=0;y<cy;y++)
        for (int x=0;x<cx;x++)
          std::swap(volume(x,y,z),volume(x,y,cz-1-z));
  }
  template <class Type>
  static bool isCanonical(Vol3D<Type> &vIn)
  {
    std::string e0s=Vol3DReorder::getOrientationRAS(vIn.currentOrientation.col0());
    std::string e1s=Vol3DReorder::getOrientationRAS(vIn.currentOrientation.col1());
    std::string e2s=Vol3DReorder::getOrientationRAS(vIn.currentOrientation.col2());
    DSPoint e0 = Vol3DReorder::codeToRASVector(e0s[0]);
    DSPoint e1 = Vol3DReorder::codeToRASVector(e1s[0]);
    DSPoint e2 = Vol3DReorder::codeToRASVector(e2s[0]);
    SILT::Mat3<float> pTranspose=SILT::Mat3<float>::fromRows(e0,e1,e2);
    return (pTranspose.m00==1 && pTranspose.m11==1 && pTranspose.m22==1);
  }
  template <class Type>
  static void reorderToRAS(Vol3D<Type> &vOut, Vol3D<Type> &vIn)
  {
    std::string e0s=Vol3DReorder::getOrientationRAS(vIn.currentOrientation.col0());
    std::string e1s=Vol3DReorder::getOrientationRAS(vIn.currentOrientation.col1());
    std::string e2s=Vol3DReorder::getOrientationRAS(vIn.currentOrientation.col2());
    DSPoint e0 = abs(Vol3DReorder::codeToRASVector(e0s[0]));
    DSPoint e1 = abs(Vol3DReorder::codeToRASVector(e1s[0]));
    DSPoint e2 = abs(Vol3DReorder::codeToRASVector(e2s[0]));
    SILT::Mat3<float> P=SILT::Mat3<float>::fromColumns(e0,e1,e2);
    SILT::Mat3<float> pTranspose=SILT::Mat3<float>::fromRows(e0,e1,e2);
    DSPoint dims(vIn.cx,vIn.cy,vIn.cz);
    DSPoint dims2=P*dims;
    vOut.setsize(dims2.x,dims2.y,dims2.z);
    vOut.currentOrientation=vIn.currentOrientation*pTranspose;
    DSPoint r2=P*DSPoint(vIn.rx,vIn.ry,vIn.rz);
    vOut.setres(r2.x,r2.y,r2.z);
    vOut.origin=vIn.origin;
    const int cx=dims.x;
    const int cy=dims.y;
    const int cz=dims.z;
    for (int z=0;z<cz;z++)
      for (int y=0;y<cy;y++)
        for (int x=0;x<cx;x++)
        {
          DSPoint p(x,y,z);
          DSPoint pNew=P*p;
          vOut((int)pNew.x,(int)pNew.y,(int)pNew.z)=vIn(x,y,z);
        }
    bool flipI=vOut.currentOrientation.m00<0;
    bool flipJ=vOut.currentOrientation.m11<0;
    bool flipK=vOut.currentOrientation.m22<0;
    DSPoint deltaVox(
          flipI ? (dims2.x-1) * vOut.rx : 0,
          flipJ ? (dims2.y-1) * vOut.ry : 0,
          flipK ? (dims2.z-1) * vOut.rz : 0);
    SILT::Mat3<float> F=SILT::Mat3<float32>::diagonal(DSPoint(flipI ? -1:1,flipJ ? -1:1,flipK ? -1:1));
    vOut.origin += vOut.currentOrientation*deltaVox; // uses the OLD orientation
    vOut.currentOrientation=vOut.currentOrientation*F;
    vOut.fileOrientation=vIn.currentOrientation;
    if (flipI) Vol3DReorder::flipX(vOut);
    if (flipJ) Vol3DReorder::flipY(vOut);
    if (flipK) Vol3DReorder::flipZ(vOut);
  }
  template <class T>
  static bool transformNIItoRAS(Vol3D<T> &vIn)
  {
    Vol3D<T> vTemp;
    vTemp.copy(vIn);
    reorderToRAS(vIn, vTemp);
    return true;
  }
};

#endif // Vol3DReorder_H
