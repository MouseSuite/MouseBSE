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

#include <iostream>
#include <fstream>
#include <DS/morph32.h>

void Morph32::erodeX32(uint32 *in, uint32 *out, const int cx, const int n)
{
  const int extra = (cx&0x1F);
  const int wpl  = ((cx>>5)-1) + (extra ? 1 : 0);
  for (int i=0;i<n;i++)
  {
    uint32 x = 0;
    uint32 y = *in;
    for (int j=0;j<wpl;j++)
    {
      uint32 z = *++in;
      *out++ = y & ((x>>31)|(y<<1)) & ((y>>1)|(z<<31));
      x = y;
      y = z;
    }
    { *out++ = y & ((x>>31)|(y<<1)) & (y>>1); ++in; }
  }
}

void Morph32::dilateX32(uint32 *in, uint32 *out, const int cx, const int n)
{
  const int extra = (cx&0x1F);
  const int wpl  = ((cx>>5)-1) + (extra ? 1 : 0);

  for (int i=0;i<n;i++)
  {
    uint32 x = 0;
    uint32 y = *in;
    for (int j=0;j<wpl;j++)
    {
      uint32 z = *++in;
      *out++ = y | (y<<1) | (y>>1) | (x>>31) | (z<<31);
      x = y;
      y = z;
    }
    { *out++ = y | (y<<1) | (y>>1) | (x>>31); ++in; }
  }
}

void Morph32::dilateY32(uint32 *in, uint32 *out, const int cx, const int cy, const int cz)
{
  const int wpl  = (cx>>5) + ((cx&0x1F)!=0);
  const int ys = cy - 1;
  uint32 *o = out;
  uint32 *i = in;
  for (int z=0;z<cz;z++)
  {
    for (int x=0;x<wpl;x++,o++,i++) o[0] = i[0] | i[wpl];
    for (int y=1;y<ys;y++)
    {
      for (int x=0;x<wpl;x++,o++,i++)
        o[0] = i[-wpl] | i[0] | i[wpl];
    }
    for (int x=0;x<wpl;x++,o++,i++) o[0] = i[-wpl] | i[0];
  }
}

void Morph32::erodeY32(uint32 *in, uint32 *out, const int cx, const int cy, const int cz)
{
  const int wpl  = (cx>>5) + ((cx&0x1F)!=0);
  const int ys = cy - 1;
  uint32 *o = out;
  uint32 *i = in;
  for (int z=0;z<cz;z++)
  {
    for (int x=0;x<wpl;x++,o++,i++) o[0] = 0;
    for (int y=1;y<ys;y++)
    {
      for (int x=0;x<wpl;x++,o++,i++)
        o[0] = i[-wpl] & i[0] & i[wpl];
    }
    for (int x=0;x<wpl;x++,o++,i++) o[0] = 0;
  }
}

void Morph32::dilateY32(uint32 *in, uint32 *inB, uint32 *out, const int cx, const int cy, const int cz)
{
  const int wpl  = (cx>>5) + ((cx&0x1F)!=0);
  const int ys = cy - 1;
  uint32 *o = out;
  uint32 *i = in;
  uint32 *ib = inB;
  for (int z=0;z<cz;z++)
  {
    for (int x=0;x<wpl;x++,o++,i++,ib++) o[0] = i[0] | ib[wpl];
    for (int y=1;y<ys;y++)
    {
      for (int x=0;x<wpl;x++,o++,i++,ib++)
        o[0] = ib[-wpl] | i[0] | ib[wpl];
    }
    for (int x=0;x<wpl;x++,o++,i++,ib++) o[0] = ib[-wpl] | i[0];
  }
}

void Morph32::erodeY32(uint32 *in, uint32 *inB, uint32 *out, const int cx, const int cy, const int cz)
{
  const int wpl  = (cx>>5) + ((cx&0x1F)!=0);
  const int ys = cy - 1;
  uint32 *o = out;
  uint32 *i = in;
  uint32 *ib= inB;
  for (int z=0;z<cz;z++)
  {
    for (int x=0;x<wpl;x++,o++) o[0] = 0;
    i += wpl; ib += wpl;
    for (int y=1;y<ys;y++)
    {
      for (int x=0;x<wpl;x++,o++,i++,ib++)
        o[0] = ib[-wpl] & i[0] & ib[wpl];
    }
    for (int x=0;x<wpl;x++,o++) o[0] = 0;
    i += wpl; ib += wpl;
  }
}

bool Morph32::dilateC(uint32 *ina, uint32 *inb)
{
  const int sz = cz;
  uint32 *a=ina, *b=inb;
  for (int i=0;i<sz;i++)
  {
    dilateX32(a,&sliceA[0],cx,cy);
    dilateY32(&sliceA[0],b,cx,cy,1);
    a += slicesize;
    b += slicesize;
  }
  b = inb;
  for (int j=0;j<slicesize;j++)
  {
    sliceA[j] = b[j];  b[j] |= b[j+slicesize];
  }
  b += slicesize;
  for (int i=1;i<sz-1;i++)
  {
    for (int j=0;j<slicesize;j++)
    {
      uint32 t = b[j];
      b[j] |= sliceA[j] | b[j+slicesize];
      sliceA[j] = t;
    }
    b += slicesize;
  }
  for (int j=0;j<slicesize;j++)
  {
    b[j] |= sliceA[j];
  }
  return true;
}

bool Morph32::erodeC (uint32 *ina, uint32 *inb)
{
  const int sz = cz;
  uint32 *a=ina, *b=inb;
  for (int i=0;i<sz;i++)
  {
    erodeX32(a,&sliceA[0],cx,cy);
    erodeY32(&sliceA[0],b,cx,cy,1);
    a += slicesize;
    b += slicesize;
  }
  b = inb;
  for (int j=0;j<slicesize;j++)
  {
    sliceA[j] = b[j];  b[j] = 0;
  }
  b += slicesize;
  for (int i=1;i<sz-1;i++)
  {
    for (int j=0;j<slicesize;j++)
    {
      uint32 t = b[j];
      b[j] &= sliceA[j] & b[j+slicesize];
      sliceA[j] = t;
    }
    b += slicesize;
  }
  for (int j=0;j<slicesize;j++)
  {
    b[j] = 0;
  }
  return true;
}
void Morph32::releaseMemory()
{
  cx=cy=cz=slicesize=0;
  sliceA=std::vector<uint32>();
  volA=std::vector<uint32>();
  volB=std::vector<uint32>();
}

void Morph32::init(int cx_, int cy_, int cz_)
{
  cx = cx_;
  cy = cy_;
  cz = cz_;
  const int extra = (cx&0x1F);
  const int wpl  = (cx>>5);
  const int wx = wpl + (extra!=0);
  if ((int)(wx*cy)!=slicesize)
  {
    slicesize = wx*cy;
    sliceA.resize(slicesize); // refactor to be container or smart pointer
    volA.resize(slicesize*cz);
    volB.resize(slicesize*cz);
  }
}

bool Morph32::dilateR(uint32 *ina, uint32 *inb)
{
  const int sz = cz;
  uint32 *a = ina;
  uint32 *b = inb;
  for (int i=0;i<sz;i++)
  {
    dilateX32(a,&sliceA[0],cx,cy);
    dilateY32(&sliceA[0],a,b,cx,cy,1);
    a += slicesize;
    b += slicesize;
  }
  b = inb;
  uint32 *c = ina + slicesize;
  for (int j=0;j<slicesize;j++)
  {
    b[j] |= c[j];
  }
  b += slicesize;
  c += slicesize;
  a = ina;
  for (int i=1;i<sz-1;i++)
  {
    for (int j=0;j<slicesize;j++)
    {
      b[j] |= a[j]|c[j];
    }
    a += slicesize;
    b += slicesize;
    c += slicesize;
  }
  for (int j=0;j<slicesize;j++)
  {
    b[j] |= a[j];
  }
  return true;
}

bool Morph32::erodeR (uint32 *ina, uint32 *inb)
{
  const int sz = cz;
  uint32 *d = inb + slicesize*(sz-1);
  for (int i=0;i<slicesize;i++) { inb[i]=0; d[i] = 0; }
  uint32 *a = ina+slicesize;
  uint32 *b = inb+slicesize;
  for (int i=1;i<sz-1;i++)
  {
    erodeX32(a,&sliceA[0],cx,cy);
    erodeY32(&sliceA[0],a,b,cx,cy,1);
    a += slicesize;
    b += slicesize;
  }
  a = ina;
  b = inb;
  uint32 *c = ina + slicesize*2;
  b += slicesize;
  // set first and last slice to 0.
  for (int i=1;i<sz-1;i++)
  {
    for (int j=0;j<slicesize;j++)
    {
      b[j] &= a[j]&c[j];
    }
    a += slicesize;
    b += slicesize;
    c += slicesize;
  }
  return true;
}

bool Morph32::dilateO2(uint32 *a, uint32 *b)
{
  dilateR(   a,&volA[0]);
  dilateR(&volA[0],&volB[0]);
  dilateC(&volB[0],&volA[0]);
  dilateC(&volA[0],   b);
  return true;
}

bool Morph32::erodeO2(uint32 *a, uint32 *b)
{
  erodeR(       a,&volA[0]);
  erodeR(&volA[0],&volB[0]);
  erodeC(&volB[0],&volA[0]);
  erodeC(&volA[0],   b);
  return true;
}
