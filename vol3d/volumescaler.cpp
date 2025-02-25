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

#include <volumescaler.h>
#include <algorithm>
#include <vector>

template <class T>
double VolumeScaler::scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<T> &vIn, const Vol3D<uint8> &vm)
{
  if (vIn.isCompatible(vm)==false) return 0;
  Vol3D<T> vTemp;
  vTemp.copy(vIn);
  vTemp.maskWith(vm);
  return scaleToUint8(vb,vTemp);
}

template <>
double VolumeScaler::scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<uint8> &vIn, const Vol3D<uint8> &vm)
{
  if (vIn.isCompatible(vm)==false) return 0;
  vb.copy(vIn);
  vb.maskWith(vm);
  return 1.0;
}

double VolumeScaler::scaleToUint8(Vol3D<uint8> &vb, const Vol3D<uint8> &vIn)
{
  vb.copy(vIn);
  return 1.0;
}

template <class FloatT>
double VolumeScaler::scaleToUint8_16bit(Vol3D<uint8> &vb, const Vol3D<FloatT> &vf)
// assumes equivalent of 16-bit range of values stored in float, e.g., a uint16 file was saved as float
{
  std::vector<int> hgram(65536);
  for (int i=0;i<65536;i++) hgram[i] = 0;
  const int ds = vf.size();
  for (int i=0;i<ds;i++) hgram[u16clamp(vf[i])]++;
  int limit = (int)(ds * 0.999);
  int maxval = 65536;
  int sum = 0;
  for (int i=0;i<65536;i++) { if ((sum+=hgram[i])>limit) { maxval = i; break; } }
  vb.makeCompatible(vf);
  uint8 *d = vb.start();
  if (maxval==0)
  {
    std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
    maxval = 1;
  }
  for (int i=0;i<ds;i++)
  {
    int v = (int)((vf[i] * 255)/maxval);
    d[i] = (v<255) ? v : 255;
  }
  return 255.0/maxval;
}

double VolumeScaler::scaleToUint8(Vol3D<uint8> &vb, const Vol3D<float32> &vf)
// float32 and float64 should use the same method
{
  const float32 maxValue = *std::max_element(vf.cbegin(),vf.cend());
  if (maxValue<65536) return scaleToUint8_16bit(vb,vf);
  if (maxValue>0)
  {
    const float32 scale = 65535/(maxValue);
    std::vector<int> hgram(65536);
    for (int i=0;i<65536;i++) hgram[i] = 0;
    const int ds = vf.size();
    for (int i=0;i<ds;i++) hgram[u16clamp(vf[i]*scale)]++;
    int limit = (int)(ds * 0.999);
    int maxval = 65536;
    int sum = 0;
    for (int i=0;i<65536;i++) { if ((sum+=hgram[i])>limit) { maxval = i; break; } }
    vb.makeCompatible(vf);
    uint8 *d = vb.start();
    if (maxval==0)
    {
      std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
      maxval = 1;
    }
    const float32 rescale = maxval / scale;
    for (int i=0;i<ds;i++)
    {
      int v = (int)((vf[i] * 255)/rescale);
      d[i] = (v<255) ? v : 255;
    }
    return 255.0/rescale;
  }
  else
  {
    std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
    vb.makeCompatible(vf);
    vb.set(0);
    return 1.0;
  }
}

double VolumeScaler::scaleToUint8(Vol3D<uint8> &vb, const Vol3D<float64> &vf)
// float32 and float64 should use the same method
{    
  const float64 maxValue = *std::max_element(vf.begin(),vf.end());
  if (maxValue<65536) return scaleToUint8_16bit(vb,vf);
  if (maxValue>0)
  {
    const float64 scale = 65535/(maxValue);
    std::vector<int> hgram(65536);
    for (int i=0;i<65536;i++) hgram[i] = 0;
    const int ds = vf.size();
    for (int i=0;i<ds;i++) hgram[u16clamp(vf[i]*scale)]++;
    int limit = (int)(ds * 0.999);
    int maxval = 65536;
    int sum = 0;
    for (int i=0;i<65536;i++) { if ((sum+=hgram[i])>limit) { maxval = i; break; } }
    vb.makeCompatible(vf);
    uint8 *d = vb.start();
    if (maxval==0)
    {
      std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
      maxval = 1;
    }
    const float64 rescale = maxval / scale;
    for (int i=0;i<ds;i++)
    {
      int v = (int)((vf[i] * 255)/rescale);
      d[i] = (v<255) ? v : 255;
    }
    return 255.0/rescale;
  }
  else
  {
    std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
    vb.makeCompatible(vf);
    vb.set(0);
    return 1.0;
  }
}

double VolumeScaler::scaleToUint8(Vol3D<uint8> &vb, const Vol3D<uint16> &vs)
{
  std::vector<int> hgram(65536);
  for (int i=0;i<65536;i++) hgram[i] = 0;
  const int ds = vs.size();
  unsigned short *s = (unsigned short *)vs.start();
  for (int i=0;i<ds;i++) hgram[s[i]]++;
  int limit = (int)(ds * 0.999); // take lower 99.9%
  int maxval = 65536;
  int sum = 0;
  for (int i=0;i<65536;i++) { if ((sum+=hgram[i])>limit) { maxval = i; break; } }
  vb.makeCompatible(vs);
  uint8 *d = vb.start();
  if (maxval==0)
  {
    std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
    maxval = 1;
  }
  for (int i=0;i<ds;i++)
  {
    int v = (s[i] * 255)/maxval;
    d[i] = (v<255) ? v : 255;
  }
  return 255.0/maxval;
}

double VolumeScaler::scaleToUint8(Vol3D<uint8> &vb, const Vol3D<sint16> &vs)
{
  std::vector<int> hgram(37268);
  for (int i=0;i<37268;i++) hgram[i] = 0;
  const int ds = vs.size();
  const auto *s = vs.start();
  for (int i=0;i<ds;i++)
  {
    hgram[(s[i]>0) ? s[i] : 0 ]++;
  }
  int limit = (int)(ds * 0.999); // take lower 99.9%
  int maxval = 32767;
  int sum = 0;
  for (int i=0;i<32768;i++) { if ((sum+=hgram[i])>limit) { maxval = i; break; } }
  vb.makeCompatible(vs);
  uint8 *d = vb.start();
  if (maxval==0)
  {
    std::cerr<<"Warning: maximum value of image is zero!"<<std::endl;
    maxval = 1;
  }
  for (int i=0;i<ds;i++)
  {
    int v = (s[i] * 255)/maxval;
    d[i] = (v<255) ? v : 255;
  }
  return 255.0/maxval;
}

template double VolumeScaler::scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<float64> &vIn, const Vol3D<uint8> &vm);
template double VolumeScaler::scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<float32> &vIn, const Vol3D<uint8> &vm);
template double VolumeScaler::scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<uint16> &vIn, const Vol3D<uint8> &vm);
template double VolumeScaler::scaleToUint8Masked(Vol3D<uint8> &vb, const Vol3D<sint16> &vIn, const Vol3D<uint8> &vm);
