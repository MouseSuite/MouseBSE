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

#include <volumeloader.h>
#include <vol3dquery.h>
#include <vol3d_t.h>
#include <vbit.h>

template <class DstT, class SrcT>
std::unique_ptr<Vol3DBase> Vol3DBase::rescaleAs(const Vol3D<SrcT> *vIn)
{
  auto vOut = std::make_unique<Vol3D<DstT>>();
  vOut->makeCompatible(*vIn);
  auto nvox=vIn->size();
  auto *dst=vOut->start();
  auto *src=vIn->start();
  for (size_t i=0;i<nvox;i++) dst[i]=vIn->scl_slope*static_cast<DstT>(src[i])+vIn->scl_inter;
  return std::move(vOut);
}

template <class SrcT>
bool Vol3DBase::rescaleInPlace(Vol3D<SrcT> *vIn) // should only be used for float types : w/C++20, this will be enforced with concepts
{
  if (!vIn) return false;
  if (vIn->typeID()==SILT::Float32||vIn->typeID()==SILT::Float32)
  {
    auto nvox=vIn->size();
    auto *src=vIn->start();
    const auto scl_slope=vIn->scl_slope;
    const auto scl_inter=vIn->scl_inter;
    for (size_t i=0;i<nvox;i++) src[i]=static_cast<SrcT>(scl_slope)*static_cast<SrcT>(src[i])+static_cast<SrcT>(scl_inter);
    vIn->scl_slope=1.0f;
    vIn->scl_inter=0.0f;
    return true;
  }
  else
  {
    std::cerr<<"warning: rescaleInPlace called with datatype "<<vIn->datatypeName()<<". dataytpe must be float32 or float64."<<std::endl;
    return false;
  }
}

std::unique_ptr<Vol3DBase> Vol3DBase::rescaleAsFloat32() const
{
  switch (typeID())
  {
    case SILT::Uint8						: return rescaleAs<float32,uint8>(static_cast<const Vol3D<uint8> *>(this));
    case SILT::Sint8						: return rescaleAs<float32,sint8>(static_cast<const Vol3D<sint8> *>(this));
    case SILT::Uint16						: return rescaleAs<float32,uint16>(static_cast<const Vol3D<uint16>*>(this));
    case SILT::Sint16						: return rescaleAs<float32,sint16>(static_cast<const Vol3D<sint16>*>(this));
    case SILT::Uint32						: return rescaleAs<float32,uint32>(static_cast<const Vol3D<uint32>*>(this));
    case SILT::Sint32						: return rescaleAs<float32,sint32>(static_cast<const Vol3D<sint32>*>(this));
//    case SILT::Float32					: return rescaleAs<float32,float32>(static_cast<const Vol3D<float32>*>(this)); // float32 and float64 should be done inplace, but this function has to return new object
//    case SILT::Float64					: return rescaleAs<float64,float64>(static_cast<const Vol3D<float64>*>(this)); // rescaling in place takes place outside of this function, and is called instead of it.
    default:
      std::cerr<<"rescaling not available for datatype "<<datatypeName()<<std::endl;
  }
  return nullptr;
}

std::unique_ptr<Vol3DBase> VolumeLoader::load(std::string ifname)
{
  std::unique_ptr<Vol3DBase> volume;
  Vol3DQuery vq;
  if (vq.query(ifname)==false) return nullptr;
  if (vq.headerType == HeaderType::DICOM) return nullptr; // DICOM not currently supported
  switch (vq.datatype)
  {
    case SILT::Uint8						: volume = std::make_unique<Vol3D<uint8>>(); break;
    case SILT::Sint8						: volume = std::make_unique<Vol3D<signed char>>(); break;
    case SILT::Uint16						: volume = std::make_unique<Vol3D<unsigned short>>(); break;
    case SILT::Sint16						: volume = std::make_unique<Vol3D<signed short>>(); break;
    case SILT::Uint32						: volume = std::make_unique<Vol3D<unsigned int>>(); break;
    case SILT::Sint32						: volume = std::make_unique<Vol3D<signed int>>(); break;
    case SILT::Float32					: volume = std::make_unique<Vol3D<float>>(); break;
    case SILT::Float64					: volume = std::make_unique<Vol3D<double>>(); break;
    case SILT::RGB8							: volume = std::make_unique<Vol3D<rgb8>>(); break;
    case SILT::Eigensystem3x3f	: volume = std::make_unique<Vol3D<EigenSystem3x3f>>(); break;
    case SILT::Vector3F					: volume = std::make_unique<Vol3D<DSPoint>>(); break;
    case SILT::Unknown :
    default:
      std::cerr<<"File datatype ("<<vq.datatype<<")is unknown."<<std::endl;
      return nullptr;
  }
  if (!volume) return nullptr;
  if (volume) volume->read(vq,Vol3DBase::RotateToRAS);
  if ((volume->scl_slope==1)&&(volume->scl_inter==0))
  {
//    std::cout<<"no scale"<<std::endl;
  }
  else
  {
    if (std::abs(volume->scl_slope)>0)
    {
      std::cerr<<"volume of type "<<volume->datatypeName()<<" has scl_slope="<<volume->scl_slope<<" and scl_inter="<<volume->scl_inter<<std::endl;
      switch (volume->typeID())
      {
        case SILT::Float32:
          Vol3DBase::rescaleInPlace(static_cast<Vol3D<float32> *>(volume.get()));
          break;
        case SILT::Float64:
          Vol3DBase::rescaleInPlace(static_cast<Vol3D<float64> *>(volume.get()));
          break;
        default:
          if (auto v=volume->rescaleAsFloat32()) volume=std::move(v);
      }
    }
  }
  return volume;
}

Vol3DInstance(sint8)
Vol3DInstance(uint8)
Vol3DInstance(sint16)
Vol3DInstance(uint16)
Vol3DInstance(sint32)
Vol3DInstance(uint32)
Vol3DInstance(float32)
Vol3DInstance(float64)
Vol3DInstance(EigenSystem3x3f)
Vol3DInstance(rgb8)

template bool Vol3D<VBit>::read(std::string, Vol3DBase::AutoRotateCode);
template bool Vol3D<VBit>::read(const Vol3DQuery &, Vol3DBase::AutoRotateCode);
template bool Vol3D<VBit>::copyCast(std::unique_ptr<Vol3DBase> &dest) const;
