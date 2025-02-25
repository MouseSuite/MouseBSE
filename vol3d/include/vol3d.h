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

#ifndef Vol3D_H
#define Vol3D_H
#include <stdint.h>
#include <vol3dbase.h>
#include <fstream>
#include <iostream>
#include <strutil.h>
#include <DS/dsheader.h>
#include <dspoint.h>
#include <point3d.h>
#include <stdio.h>
#include <silttypes.h>
#include <eigensystem3x3.h>
#include <rgb8.h>
#include <vector>

namespace SILT { class izstream; }

template <class Datatype>
class Vol3D : public Vol3DBase {
public:
  Vol3D() : Vol3DBase() {}
  virtual ~Vol3D() override
  {
    cx = 0; cy = 0; cz = 0;
  }
  void releaseMemory()
  {
    cx=cy=cz=0;
    std::vector<Datatype>().swap(data);
  }
  size_t readDataStream(SILT::izstream &ifile);
  virtual bool readNifti(std::string ifname, AutoRotateCode autoRotate=RotateToRAS) override;
  virtual bool read(std::string ifname, Vol3DBase::AutoRotateCode autoRotate=RotateToRAS) override;
  virtual bool read(const Vol3DQuery &query, AutoRotateCode autoRotate=RotateToRAS) override; // assumes query has already been run
  virtual bool write (std::string ifile) override;
  // data info
  SILT::DataType typeID() const override { return SILT::Unknown; } // specialize for given types
  virtual int analyzeTypeID() const override { return DT_UNKNOWN; }
  virtual int niftiTypeID() const override { return DT_UNKNOWN; }
  dim_type dimX() const override { return cx; }
  dim_type dimY() const override { return cy; }
  dim_type dimZ() const override { return cz; }
  float resX() const override { return rx; }
  float resY() const override { return ry; }
  float resZ() const override { return rz; }
  // data operations
  virtual bool maskWith(const Vol3D<unsigned char> &vMask) override;
  const Datatype *slice(const dim_type n) const { return &data[size_t(cx)*size_t(cy)*size_t(n)]; }
  Datatype *slice(const dim_type n) { return &data[size_t(cx)*size_t(cy)*size_t(n)]; }
  Vol3DBase *duplicate() const override
  {
    Vol3D<Datatype> *newVol = new Vol3D<Datatype>;
    newVol->copy(*this);
    return newVol;
  }
  size_t size() const override { return data.size(); }
  auto begin() const { return data.begin(); }
  auto end() const { return data.end(); }
  auto cbegin() const { return data.cbegin(); }
  auto cend() const { return data.cend(); }
  void set(const Datatype& val)
  {
    std::fill(data.begin(),data.end(),val);
  }
  // compatibility with other volumes
  int databits() const override { return sizeof(Datatype)*8; }
  template<class S> bool isCompatible(const Vol3D<S> &vol) const
  {
    return (cx == vol.cx) && (cy==vol.cy) && (cz==vol.cz);
  }
  bool isCompatible(const Vol3DBase *vol) const
  {
    return (cx == vol->cx) && (cy==vol->cy) && (cz==vol->cz);
  }
  dim_type index(const dim_type x, const dim_type y, const dim_type z) const
  {
    return x + (y + z*cy)*cx;
  }
  dim_type index(const Point3D<int> &p) const
  {
    return index(p.x,p.y,p.z);
  }
  Datatype &operator[](const dim_type i)
  {
    return data[i];
  }
  Datatype operator[](const dim_type i) const
  {
    return data[i];
  }
  Datatype &operator()(const Point3D<int> &p)
  {
    return data[index(p)];
  }
  Datatype operator()(const Point3D<int> &p) const
  {
    return data[index(p)];
  }
  Datatype &operator()(const dim_type x, const dim_type y, const dim_type z)
  {
    return data[index(x,y,z)];
  }
  Datatype operator()(const dim_type x, const dim_type y, const dim_type z) const
  {
    return data[index(x,y,z)];
  }
  int minVal() const override { return 0; }
  int maxVal() const override { return 0; }
  Datatype *start() { return &data[0]; }
  Datatype const *start() const { return &data[0]; }
  auto *craw32() const { return reinterpret_cast<const uint32 *>(&data[0]); }
  auto *raw32() { return reinterpret_cast<uint32 *>(&data[0]); }
  auto *raw64()  { return reinterpret_cast<uint64 *>(&data[0]); }
  auto *craw64() const { return reinterpret_cast<const uint64 *>(&data[0]); }
  bool setsize(const dim_type cx_, const dim_type cy_, const dim_type cz_) override
  {
    const size_t nElements = size_t(cx_) * size_t(cy_) * size_t(cz_);
    {
      data.resize(nElements);
      if (data.size()==nElements)
      {
        cx = cx_;
        cy = cy_;
        cz = cz_;
      }
      else
      {
        std::cout<<__FILE__<<" : "<<__LINE__<<" Allocation failed."<<std::endl;
        return false;
      }
    }
    return true;
  }
  template <class S> bool copyInfo(const Vol3D<S> &vol)
  // TODO: this does not copy orientation codes or adjust position
  {
    rx = vol.rx;
    ry = vol.ry;
    rz = vol.rz;
    return true;
  }
  bool encode(const Vol3D<uint8> &mask);  // use only for VBit!!!
  bool decode(Vol3D<uint8> &mask);        // use only for VBit!!!
  bool copy(const Vol3D<Datatype> &source);
  bool copyCast(std::unique_ptr<Vol3DBase> &dest) const override; // copies the data to a new vol3d
private:
  Vol3D(Vol3D<Datatype> &v)	// Being private forces the object to be passed BY REFERENCE ONLY!!!
  {                             // Otherwise, data will be destroyed when the temp copy object is destroyed.
    if (&v!=this)
    {
      copy(v);
    }
  }
protected:
  std::vector<Datatype> data;
};

template<> inline int Vol3D<uint8>::minVal() const { return 0; }
template<> inline int Vol3D<uint8>::maxVal() const { return 255; }
template<> inline int Vol3D<sint16>::minVal() const { return 0; }
template<> inline int Vol3D<sint16>::maxVal() const { return 32767; }
template<> inline int Vol3D<uint16>::minVal() const { return 0; }
template<> inline int Vol3D<uint16>::maxVal() const { return 65535; }

inline Vol3DBase::dim_type wordsPerLine(Vol3DBase::dim_type cx)
{
  auto wpl = (cx/32);
  if (wpl*32 != cx) wpl++;
  return wpl;
}

template <class T>
inline bool Vol3D<T>::copy(const Vol3D<T> &vSource)
{
  if (!makeCompatible(vSource)) return false;
  auto src = vSource.cbegin();
  for (auto &v : data) v=*src++;
  return true;
}

template <class DT, class DS> bool copy(Vol3D<DT> &vDest, const Vol3D<DS> &vSource)
{
  if (!vDest.makeCompatible(vSource)) return false;
  const size_t ds = vSource.size();
  DT *d = vDest.start();
  DS *s = vSource.start();
  for (size_t i=0;i<ds;i++) d[i] = s[i];
  return true;
}


template<> inline int Vol3D<uint8>::niftiTypeID() const { return DT_UNSIGNED_CHAR; }
template<> inline int Vol3D<sint16>::niftiTypeID() const { return DT_SIGNED_SHORT; }
template<> inline int Vol3D<sint32>::niftiTypeID() const { return DT_SIGNED_INT; }
template<> inline int Vol3D<sint8>::niftiTypeID() const { return DT_UNSIGNED_CHAR; }
template<> inline int Vol3D<uint16>::niftiTypeID() const { return DT_UINT16; }
template<> inline int Vol3D<uint32>::niftiTypeID() const { return DT_UINT32; }
template<> inline int Vol3D<float32>::niftiTypeID() const { return DT_FLOAT; }
template<> inline int Vol3D<float64>::niftiTypeID() const { return DT_DOUBLE; }
template<> inline int Vol3D<rgb8>::niftiTypeID() const { return DT_RGB; }

template<> inline int Vol3D<uint8>::analyzeTypeID() const { return DT_UNSIGNED_CHAR; }
template<> inline int Vol3D<sint16>::analyzeTypeID() const { return DT_SIGNED_SHORT; }
template<> inline int Vol3D<sint32>::analyzeTypeID() const { return DT_SIGNED_INT; }
template<> inline int Vol3D<sint8>::analyzeTypeID() const { return DT_UNSIGNED_CHAR; }
template<> inline int Vol3D<uint16>::analyzeTypeID() const { return DT_SIGNED_SHORT; }
template<> inline int Vol3D<uint32>::analyzeTypeID() const { return DT_SIGNED_INT; }
template<> inline int Vol3D<float32>::analyzeTypeID() const { return DT_FLOAT; }
template<> inline int Vol3D<float64>::analyzeTypeID() const { return DT_DOUBLE; }
template<> inline int Vol3D<rgb8>::analyzeTypeID() const { return DT_RGB; }

template<> inline SILT::DataType Vol3D<uint8>::typeID() const { return SILT::Uint8; }
template<> inline SILT::DataType Vol3D<uint16>::typeID() const { return SILT::Uint16; }
template<> inline SILT::DataType Vol3D<sint16>::typeID() const { return SILT::Sint16; }
template<> inline SILT::DataType Vol3D<sint32>::typeID() const { return SILT::Sint32; }
template<> inline SILT::DataType Vol3D<sint8>::typeID() const { return SILT::Sint8; }
template<> inline SILT::DataType Vol3D<uint32>::typeID() const { return SILT::Uint32; }
template<> inline SILT::DataType Vol3D<float32>::typeID() const { return SILT::Float32; }
template<> inline SILT::DataType Vol3D<float64>::typeID() const { return SILT::Float64; }
template<> inline SILT::DataType Vol3D<rgb8>::typeID() const { return SILT::RGB8; }
template<> inline SILT::DataType Vol3D<EigenSystem3x3f>::typeID() const { return SILT::Eigensystem3x3f; }

#include <vol3dutils.h>

#endif
