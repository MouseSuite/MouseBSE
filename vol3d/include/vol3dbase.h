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

#ifndef Vol3DBase_H
#define Vol3DBase_H

#include <iostream>
#include <silttypes.h>
#include <mat3.h>
#include <niftiinfo.h>
#include <vol3ddatatypes.h>
#include <memory>

template<class S> class Vol3D;

struct nifti_1_header;
class Vol3DQuery;

class Vol3DBase {
public:
  typedef size_t dim_type;
  Vol3DBase();
  virtual ~Vol3DBase();
  virtual bool copyCast(std::unique_ptr<Vol3DBase> &/* dest */) const { return false; }
  virtual dim_type size() const=0;
  std::string datatypeName() const;
// I/O
  enum AutoRotateCode { NoRotate=0,RotateToRAS=1 };
  static bool noRotate; // global lock against autorotate
  bool scanQForm(const nifti_1_header &header); // load voxel dimensions and orientation
  bool scanSForm(const nifti_1_header &header); // load voxel dimensions and orientation
  bool setQForm(nifti_1_header &header) const; // set voxel dimensions and orientation
  bool setSForm(nifti_1_header &header) const; // set voxel dimensions and orientation
  bool setHeader(nifti_1_header &header) const; // populate values in nifti header
  virtual bool readNifti(std::string /*ifname*/, AutoRotateCode=NoRotate) { return false; }
  virtual bool read(std::string /* ifname */, AutoRotateCode=NoRotate) { return false; }
  virtual bool read(const Vol3DQuery &/* query */, AutoRotateCode=NoRotate) { return false; } // assumes query has already been run
  virtual bool write (std::string /* ifname */ ) { return false; }
// type information
  virtual int minVal() const { return 0; }
  virtual int maxVal() const { return 0; }
  virtual SILT::DataType typeID() const { return SILT::Unknown; }
  virtual int analyzeTypeID() const { return 0; }
  virtual int niftiTypeID() const { return 0; }
  virtual int databits() const { return 0; }
  virtual dim_type dimX() const { return 0; }
  virtual dim_type dimY() const { return 0; }
  virtual dim_type dimZ() const { return 0; }
  virtual float resX() const { return 0; }
  virtual float resY() const { return 0; }
  virtual float resZ() const { return 0; }
  virtual Vol3DBase *duplicate() const =0;
// operations
  virtual bool setsize(const dim_type /* cx */, const dim_type /* cx */, const dim_type /* cx */) { return false; }
  virtual bool setres(const float rx_, const float ry_, const float rz_) { rx=rx_; ry=ry_; rz=rz_; return true; }
  virtual bool mask(Vol3DBase & /* vMask */) { return false; }
  virtual bool maskWith(const Vol3D<uint8> &vMask)=0;
  virtual std::unique_ptr<Vol3DBase> rescaleAsFloat32() const;
  bool dimensionsMatch(const Vol3DBase &vol) const
  {
    return (cx == vol.cx) && (cy==vol.cy) && (cz==vol.cz);
  }
  bool makeCompatible(const Vol3DBase &vol);
  // data
  SILT::NIFTIInfo niftiInfo;
  dim_type cx,cy,cz;
  float rx,ry,rz;
  DSPoint origin;
  float scl_slope=0; // initialized to zero -- non-zero if read from file.
  float scl_inter=0;
  SILT::Mat3<float32> fileOrientation;        // the orientation matrix for the file, as loaded
                                              // identity should correspond to LPI coordinates
  SILT::Mat3<float32> currentOrientation;     // the orientation matrix for the file, as loaded
                                              // identity should correspond to LPI coordinates
  SILT::Mat3<float32>  transformCurrenttoFile;// a transform to go from the internal, working coordinates
                                              // to the coordinates of the file as loaded
                                              // this transform should be used to reorient
                                              // the file before saving it
                                              // these values should all be 1 or -1 or 0!!!
  std::string description;
  std::string filename;
  template <class SrcT> static bool rescaleInPlace(Vol3D<SrcT> *vIn); // TODO: consider moving to derived class.
  template <class DstT, class SrcT> static std::unique_ptr<Vol3DBase> rescaleAs(const Vol3D<SrcT> *vIn);
};

#endif
