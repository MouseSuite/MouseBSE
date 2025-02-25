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

#ifndef Vol3DOps_H
#define Vol3DOps_H

#include <vol3d.h>

//! \brief Templated class for computing various image volume related operations
//! \details This class presently computes volume means and masked volume means. Applicable only to scalar types.
//! \author david w. shattuck
//! \date 30 April 2010
class Vol3DOps {
public:
  template <class T> static double meanT(const Vol3D<T> &volume);	//!< compute the mean of the input volume
  template <class T> static double meanT(const Vol3D<T> &volume, const Vol3D<uint8> &maskVolume); //!< compute the mean of the input volume in the region where maskVolume is non-zero
  static double mean(const Vol3DBase *volume); //!< compute the mean of the input volume. Invokes appropriate template function.
  static double mean(const Vol3DBase *volume, const Vol3D<uint8> &maskVolume); //!< compute the mean of the input volume in the region where maskVolume is non-zero.  Invokes appropriate template function.
};


template<class T>
bool applyMask(Vol3D<T> &vOut, Vol3D<uint8> &vMask)
{
  if (!vOut.isCompatible(vMask)) return false;
  const int ds = vOut.size();
  for (int i=0;i<ds;i++) vOut[i] = (vMask[i]) ? vOut[i] : 0;
  return true;
}

#endif
