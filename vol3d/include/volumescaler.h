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

#ifndef VolumeScaler_H
#define VolumeScaler_H

#include <vol3d.h>

//! \brief This class rescales image volumes to fit their dynamic range into an 8-bit volume.
//! \details Scaling is presently performed such that the brightest 0.1% of the image is mapped to 255.
//!          Applying to an 8-bit image results in a straight copy of the input data.
//! \author david w. shattuck
//! \date 30 April 2010

// modified 30 Aug 2011 to return scale factor used
// modified 23 Dec 2012 to fix max range of uint16
class VolumeScaler {
public:
  template <class T> static double scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<T> &vIn, const Vol3D<uint8> &vm);
  template <class FloatT> static double scaleToUint8_16bit(Vol3D<uint8> &vb, const Vol3D<FloatT> &vf);
  static double scaleToUint8(Vol3D<uint8> &vb, const Vol3D<uint8> &vIn);
  static double scaleToUint8(Vol3D<uint8> &vb, const Vol3D<uint16> &vs);
  static double scaleToUint8(Vol3D<uint8> &vb, const Vol3D<sint16> &vs);
  static double scaleToUint8(Vol3D<uint8> &vb, const Vol3D<float32> &vf);
  static double scaleToUint8(Vol3D<uint8> &vb, const Vol3D<float64> &vf);
  static uint16 u16clamp(const float32 f) { return (f<65535) ? ((f>=0) ? (uint16)f : 0) : 65535; }
  static uint16 u16clamp(const float64 f) { return (f<65535) ? ((f>=0) ? (uint16)f : 0) : 65535; }
};

#endif
