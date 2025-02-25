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

#ifndef MarrHildrethEdgeDetector_H
#define MarrHildrethEdgeDetector_H

#include <iostream>
#include <math.h>
#include <numeric>
#include <vol3d.h>
#include <strideiterator.h>

template <class T>
class MarrHildrethEdgeDetector {
public:
  MarrHildrethEdgeDetector() : sigma(0.75f), blocksize(30)
  {
  }
  float sigma;
  int blocksize; // compute edge detection in blocks of slices
  inline int Idx(const int z, const int y, const int x) { return z*zStride + y * yStride + x; }
  int zStride;
  int yStride;
  bool detect(const Vol3D<T> &vIn, Vol3D<uint8> &vOut)
  {
    const int cx = vIn.cx;
    const int cy = vIn.cy;
    const int cz = vIn.cz;
    yStride = cx;
    zStride = cx*cy;
    const int slicesize = zStride;
    int stepsize = blocksize;
    vOut.makeCompatible(vIn);
    vOut.set(0);
    const int Imin = 0;
    const int iMax = cy;
    const int jMin = 0;
    const int jMax = cx;
// Gaussian
    const int winSize = windowSize(sigma);
    const int halfWindow = winSize/2 + 1;
    int			firstSlice=0, lastSlice=0, nSlices=0;
    int			kMax=0;

    if (cz <= stepsize)
    {
      if ((cz + halfWindow) > stepsize)
      {
        stepsize += halfWindow;
      }
    }
    int outindex = 0;

    std::vector<double> Gauss;
    std::vector<double> Gauss2p;
    computeGaussianFilters(Gauss, Gauss2p, sigma, winSize);

    const int jStart = jMin+halfWindow-1;
    const int jStop  = jMax-halfWindow+1;
    const int dataSize = (stepsize + 2 * halfWindow) * iMax * jMax;
    std::vector<float> sliceA(iMax * jMax);
    std::vector<float> sliceB(iMax * jMax);
    std::vector<float> sliceV (cz + 2 * winSize);
    std::vector<float> sliceV1(cz + 2 * winSize);
    std::vector<float> sliceV2(cz + 2 * winSize);
    std::vector<float> imageOut(dataSize);
    std::vector<float> imageTemp(dataSize);
    std::fill(sliceA.begin(),sliceA.end(),0.0f);
    for (int nz=1; nz<cz; nz=lastSlice)
    {
      std::fill(imageOut.begin(),imageOut.end(),0.0f);
      std::fill(imageTemp.begin(),imageTemp.end(),0.0f);
      if ( nz==1 && nz+stepsize-winSize< cz)
      {
        firstSlice = 1;
        lastSlice = firstSlice+stepsize-1;
      }
      else if ( nz+stepsize-winSize<cz )
      {
        firstSlice = lastSlice-winSize;
        lastSlice = firstSlice+stepsize-1;
      }
      else if ( nz+stepsize-winSize>=cz && nz!=1)
      {
        firstSlice = lastSlice-winSize;
        lastSlice = cz;
      }
      else
      {
        firstSlice = 1;
        lastSlice = cz;
      }
      nSlices = lastSlice - firstSlice + 1;
      const int Kmin = 0;
      kMax = nSlices;
      if (nz == 1 || lastSlice == cz)
        nSlices += halfWindow;
      if (nz == 1 && lastSlice == cz)
      {
        kMax += halfWindow;
        nSlices += halfWindow;
      }
      {
        int position = (firstSlice - 1) * slicesize;
        int Offset = 0;
        const T *iptr = vIn.start() + position;
        int zoffset = 0;
        int zstop = kMax;
        int zlast = kMax;
        if (nz==1)
        {
          if (lastSlice != cz)
          {
            zstop += halfWindow;
          }
          zlast += halfWindow;
          zoffset = halfWindow;
          kMax += halfWindow;
        }
        Offset = slicesize*zoffset;
        if (lastSlice == cz)
        {
          zlast = zstop + halfWindow;
          kMax += halfWindow;
        }
        for (int z = 0; z<zlast; z++) sliceV[z] = 0.0f;
        int zmax = (cz) - (firstSlice - 1);
        if ((zstop - zoffset)>zmax) zstop = zoffset + zmax;
        for (int i=Imin+halfWindow-1; i<iMax-halfWindow+1; i++)
        {
          for (int j=jStart; j<jStop; j++)
          {
            const int ixx = Idx(0,i,j);
            for (int z=zoffset,index = ixx; z<zstop; z++, index += slicesize)
            {
              sliceV[z] = (float)iptr[index];
            }
            for (int k=Kmin+halfWindow-1; k<kMax-halfWindow+1; k++)
            {
              sliceV1[k] = (float)std::inner_product(Gauss.begin(),Gauss.end(),sliceV.begin()+k-(halfWindow-1),0.0);
              sliceV2[k] = (float)std::inner_product(Gauss2p.begin(),Gauss2p.end(),sliceV.begin()+k-(halfWindow-1),0.0);
            }
            for (int z=0,index = ixx;z<zlast;z++,index += slicesize)
            {
              imageOut [index] = sliceV1[z];
              imageTemp[index] = sliceV2[z];
            }
          }
        }
      }
      // calculate G[z]*I and G"[z]*I
      // this step performed above during the copying of the data
      // calculate G[y]*G[z]*I and G"[y]*G[z]*I
      std::fill(sliceA.begin(),sliceA.end(),0.0f);
      std::fill(sliceB.begin(),sliceB.end(),0.0f);
      for (int k=Kmin+halfWindow-1; k<kMax-halfWindow+1; k++)
      {
        for (int i=Imin+halfWindow-1; i<iMax-halfWindow+1; i++)
        {
          int index = Idx(k, i, jStart);
          int index2 = Idx(0, i, jStart);
          const int shift = yStride * (-halfWindow + 1);
          for (int j=jStart; j<jStop; j++, index++,index2++)
          {
            sliceB[index2] = (float)std::inner_product(Gauss.begin(),Gauss.end(),stride_iter<float *>(&imageOut[index + shift],yStride),0.0);
            sliceA[index2] = (float)std::inner_product(Gauss2p.begin(),Gauss2p.end(),stride_iter<float *>(&imageOut[index + shift],yStride),0.0);
          }
        }
        // calculate (G[x]*G"[y]*G[z]*I)
        for (int i=Imin+halfWindow-1; i<iMax-halfWindow+1; i++)
        {
          int index = Idx(k,i,jMin+halfWindow-1);
          int index2 = Idx(0,i,jMin);
          for (int j=jStart; j<jStop; j++, index++, index2++)
          {
            imageOut[index] = (float)std::inner_product(Gauss.begin(),Gauss.end(),sliceA.begin()+index2,0.0);
          }
        }
        // calculate (G"[x]*G[y]*G[z]*I)
        for (int i=Imin+halfWindow-1; i<iMax-halfWindow+1; i++)
        {
          int index = Idx(k, i, jStart);
          int index2 = Idx(0,i,jStart);
          const int shift = -halfWindow + 1;
          for (int j=jStart; j<jStop; j++, index++, index2++)
          {
            imageOut[index] += (float)std::inner_product(Gauss2p.begin(),Gauss2p.end(),sliceB.begin()+index2+shift,0.0);
          }
        }
      }
      // calculate (G[y]*G"[z]*I)
      std::fill(sliceA.begin(),sliceA.end(),0.0f);
      for (int k=Kmin+halfWindow-1; k<kMax-halfWindow+1; k++)
      {
        for (int j=jStart; j<jStop; j++)
        {
          int index = Idx(k, Imin+halfWindow-1, j);
          int index2 = Idx(0, Imin+halfWindow-1, j);
          const int shift = (-halfWindow + 1) * yStride;
          for (int i=Imin+halfWindow-1; i<iMax-halfWindow+1; i++, index+= yStride, index2 += yStride)
          {
            sliceA[index2] = (float)std::inner_product(Gauss.begin(),Gauss.end(),stride_iter<float *>(&imageTemp[index + shift],yStride),0.0);
          }
        }
        // calculate (G[x]*G[y]*G"[z]*I)
        for (int i=Imin+halfWindow-1; i<iMax-halfWindow+1; i++)
        {
          int index = Idx(k, i, jStart);
          int index2 = Idx(0, i, jStart);
          const int shift = (-halfWindow + 1);
          for (int j=jStart; j<jStop; j++, index++, index2++)
          {
            imageOut[index] += (float)std::inner_product(Gauss.begin(),Gauss.end(),sliceA.begin()+index2 + shift,0.0);
          }
        }
      }
      // calculate (G[x]*G"[y]*G[z]*I)
      // locate zero-crossingss
      int zMax = cz;
      for (int k=Kmin+halfWindow; k<kMax-halfWindow; k++)
      {
        if ((outindex / zStride)>=zMax) break;
        outindex += halfWindow * yStride;
        for (int i=Imin+halfWindow; i<iMax-halfWindow; i++)
        {
          outindex += halfWindow;
          int index = Idx(k, i, jMin+halfWindow);
          const float *img = &imageOut[index];
          int n;
          for (int j=jMin+halfWindow; j<jMax-halfWindow; j++, img++)
          {
            if ((img[0]) < 0.0)
            {
              if (  img[-1]>0
                    ||img[ 1]>0
                    ||img[-yStride - 1]>0
                    ||img[-yStride + 1]>0
                    ||img[ yStride - 1]>0
                    ||img[ yStride + 1]>0
                    ||img[-yStride    ]>0
                    ||img[ yStride    ]>0
                    ||img[-zStride - 1]>0
                    ||img[-zStride + 1]>0
                    ||img[-zStride - yStride - 1]>0
                    ||img[-zStride - yStride + 1]>0
                    ||img[-zStride + yStride - 1]>0
                    ||img[-zStride + yStride + 1]>0
                    ||img[-zStride - yStride]>0
                    ||img[-zStride + yStride]>0
                    ||img[ zStride - 1]>0
                    ||img[ zStride + 1]>0
                    ||img[ zStride - yStride - 1]>0
                    ||img[ zStride - yStride + 1]>0
                    ||img[ zStride + yStride - 1]>0
                    ||img[ zStride + yStride + 1]>0
                    ||img[ zStride - yStride]>0
                    ||img[ zStride + yStride]>0
                    ||img[-zStride ]>0
                    ||img[ zStride ]>0)
                n = 0;
              else
                n = 255;
            }
            else
              n = 255;
            vOut[outindex++] = n;
          }
          outindex += halfWindow;
        }
        outindex += halfWindow * yStride;
      }
    }
    return true;
  }
  void computeGaussianFilters(std::vector<double> &gauss, std::vector<double> &gauss2p, const double sigma, const int winSize)
  {
    gauss.resize(winSize);
    gauss2p.resize(winSize);
    const int halfWin = winSize/2 + 1;
    for (int k=0; k<halfWin; k++)
    {
      float r2 = float((k-halfWin+1)*(k-halfWin+1));
      gauss[k] = (1/(sqrt(2*M_PI)*sigma))*exp(r2/(-2*sigma*sigma));
      gauss[winSize-1-k] = gauss[k];
      gauss2p[k] = (1/(sqrt(2*M_PI)*sigma*sigma*sigma))*exp(r2/(-2*sigma*sigma))*(1-r2/(sigma*sigma));
      gauss2p[winSize-1-k] = gauss2p[k];
    }
  }
  int windowSize(const double sigma)
// Selection of window size based on paper by Malik, et al.
  {
    double remainder = fmod((double)(3.0*sigma),(double)1.0);
    if (remainder <= 0.5)
      return 1 + 2 * (int)(floor((double)(3.0*sigma)));
    else
      return 1 + 2 * (int)(ceil((double)(3.0*sigma)));
  }
};

#endif
