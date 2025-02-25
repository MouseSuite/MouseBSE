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

#include <vol3dops.h>

template <class T>
double Vol3DOps::meanT(const Vol3D<T> &vs)
{
  double sum=0;
  const auto *d = vs.start();
  const int n = vs.size();
  for (int i=0;i<n;i++) sum += d[i];
  return double(sum)/n;
}

template<class T>
double Vol3DOps::meanT(const Vol3D<T> &vs, const Vol3D<uint8> &vm)
{
  double sum=0;
  const auto *d = vs.start();
  const auto *m = vm.start();
  const int n = vs.size();
  int count=0;
  for (int i=0;i<n;i++)
  {
    if (m[i])
    {
      sum += d[i];
      count++;
    }
  }
  return double (sum)/count;
}

double Vol3DOps::mean(const Vol3DBase *vIn, const Vol3D<uint8> &vm)
{
  switch (vIn->typeID())
  {
    case SILT::Uint8 : return meanT(*static_cast<const Vol3D<uint8 > *>(vIn),vm); break;
    case SILT::Sint8 : return meanT(*static_cast<const Vol3D<sint8 > *>(vIn),vm); break;
    case SILT::Uint16: return meanT(*static_cast<const Vol3D<uint16> *>(vIn),vm); break;
    case SILT::Sint16: return meanT(*static_cast<const Vol3D<sint16> *>(vIn),vm); break;
    case SILT::Float32: return meanT(*static_cast<const Vol3D<float32> *>(vIn),vm); break;
    case SILT::Float64: return meanT(*static_cast<const Vol3D<float64> *>(vIn),vm); break;
    default:
      std::cerr<<"requested mean (masked) for datatype "<<vIn->typeID()<<std::endl;
  }
  return 0;
}

double Vol3DOps::mean(const Vol3DBase *vIn)
{
  switch (vIn->typeID())
  {
    case SILT::Uint8 : return meanT(*static_cast<const Vol3D<uint8 > *>(vIn)); break;
    case SILT::Sint8 : return meanT(*static_cast<const Vol3D<sint8 > *>(vIn)); break;
    case SILT::Uint16: return meanT(*static_cast<const Vol3D<uint16> *>(vIn)); break;
    case SILT::Sint16: return meanT(*static_cast<const Vol3D<sint16> *>(vIn)); break;
    case SILT::Float32: return meanT(*static_cast<const Vol3D<float32> *>(vIn)); break;
    case SILT::Float64: return meanT(*static_cast<const Vol3D<float64> *>(vIn)); break;
    default:
      std::cerr<<"requested mean for datatype "<<vIn->typeID()<<std::endl;
  }
  return 0;
}
