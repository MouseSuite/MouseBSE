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

#ifndef AnisotropicDiffusionFilter_H
#define AnisotropicDiffusionFilter_H

#include <vol3d.h>

class AnisotropicDiffusionFilter {
public:
  template <class T> inline T square(const T &t) { return t*t; }
  AnisotropicDiffusionFilter(const int nIterations_=3, const float diffusion_=25.0f, const float timestep_=0.125f) :
    nIterations(nIterations_), diffusion(diffusion_), timestep(timestep_)
  {
  }
  bool filter(Vol3D<uint8> &vOut, const Vol3D<uint8> &vIn, int verbosity);
protected:
  int nIterations;
  float diffusion;
  float timestep;
};

#endif
