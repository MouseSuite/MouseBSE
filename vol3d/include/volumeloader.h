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

#ifndef VolumeLoader_H
#define VolumeLoader_H

class Vol3DBase;
#include <string>
#include <memory>

//! \brief A class for loading image volumes.
//! \details The loader function can open various types of image data volume files.
//!          The files must be in the formats supported by the Vol3D class (nifti or Analyze)
//! \author david w. shattuck
//! \date 30 April 2010
class VolumeLoader {
public:
  VolumeLoader() {}
  static std::unique_ptr<Vol3DBase> load(std::string ifname);	//!< Load the image volume located at ifname. Returns 0 if image could not be loaded.
};

#endif
