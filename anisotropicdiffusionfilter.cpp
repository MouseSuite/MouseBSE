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

#include "anisotropicdiffusionfilter.h"

bool AnisotropicDiffusionFilter::filter(Vol3D<uint8> &vOut, const Vol3D<uint8> &vIn, int verbosity)
{
  std::vector<float> C(3*255*255+1);
  for(int i=0; i<=3*255*255; i++)
    C[i] = timestep*(float)exp((double)(-i) /(double)(diffusion*diffusion));
  const int cx = vIn.cx;
  const int cy = vIn.cy;
  const int cz = vIn.cz;
  const int Kmax  = cz;
  const int Imax  = cy - 3;
  const int Jmax  = cx - 3;
  const int slicesize = cx*cy;
  const int zStride = slicesize;
  const int yStride = cx;
  const int datasize  = vIn.size();
  const int z2 = 2 * slicesize;
  float Ce=0, Cw=0, Cn=0, Cs=0, Ct=0, Cb=0;
  uint8 *cptr=0;

  uint8 *In  = (new uint8[datasize + 2 * slicesize]);
  uint8 *Out = (new uint8[datasize + 2 * slicesize]);
  // zero-pad the volume
  memset(In, 0, slicesize);
  memcpy(In + slicesize,vIn.start(),datasize);
  memset(In + slicesize + datasize, 0, slicesize);

  memset(Out, 0, slicesize+datasize+slicesize); // required to pass valgrind checks

  for (int n=0; n<nIterations; n++)
  {
    if (verbosity>1)
    {
      std::cout<<"Anisotropic Filter "<<n+1<<std::endl;
    }
    int index2 = vIn.index(1,3,3);    // as seen here, where the indices are
    cptr = In + index2;								// offset by one.  This could be fixed.
    for (int i=3; i<Imax; i++)
    {
      for (int j=3; j<Jmax; j++)
      {
        uint8 c0 = *cptr;
        Ce=C[(square(int(cptr[2]                           -c0))
           +square(int(cptr[yStride + 1]-cptr[- yStride + 1]))
            +square(int(cptr[zStride + 1]-cptr[- zStride + 1])))];
        Cw=C[(square(int(c0               -cptr[-2           ]))
           +square(int(cptr[yStride - 1]-cptr[-yStride - 1]) )
            +square(int(cptr[zStride - 1]-cptr[-zStride - 1]) ))];
        Cn=C[(square(int(cptr[2*yStride  ]-                 c0))
           +square(int(cptr[  yStride+1]-cptr[ yStride-1]))
            +square(int(cptr[ zStride+yStride]-cptr[-zStride+yStride])))];
        Cs=C[(square(int(c0-cptr[-2*yStride]))
           +square(int(cptr[-yStride+1]-cptr[-yStride-1]))
            +square(int(cptr[ zStride-yStride]-cptr[-zStride-yStride])))];
        Ct=C[(square(int(cptr[z2]-c0))
              +square(int(cptr[ zStride+yStride]-cptr[ zStride-yStride]))
            +square(int(cptr[ zStride+1]-cptr[ zStride-1])))];
        Cb=C[(square(int(c0))
              +square(int(cptr[-zStride+yStride]-cptr[-zStride-yStride]))
            +square(int(cptr[-zStride+1]-cptr[-zStride-1])))];
        Out[index2++] = c0 + (char)(
                          (Ce*int(cptr[2]-c0)
                          -Cw*int(c0-cptr[-2])
                        +Cn*int(cptr[2*yStride]-c0)
            -Cs*int(c0-cptr[-2*yStride])
            +Ct*int(cptr[z2]-c0)
            -Cb*int(c0)));
        cptr++;
      }
      cptr += 6;
      index2 += 6;
    }
    for (int k=1; k<Kmax-1; k++)
    {
      int index2 = vIn.index(3,3,k+1);
      cptr = In + index2;
      for (int i=3; i<Imax; i++)
      {
        for (int j=3; j<Jmax; j++)
        {
          uint8 c0 = *cptr;
          Ce= C[square(int(cptr[2]                           -c0))
              +square(int(cptr[yStride + 1]-cptr[- yStride + 1]))
              +square(int(cptr[zStride + 1]-cptr[- zStride + 1]))];
          Cw=C[square(int(c0               -cptr[-2           ]))
              +square(int(cptr[yStride - 1]-cptr[-yStride - 1]))
              +square(int(cptr[zStride - 1]-cptr[-zStride - 1]))];
          Cn=C[square(int(cptr[2*yStride  ]-                 c0))
              +square(int(cptr[  yStride+1]-cptr[ yStride-1]))
              +square(int(cptr[ zStride+yStride]-cptr[-zStride+yStride]))];
          Cs=C[square(int(c0-cptr[-2*yStride]))
              +square(int(cptr[-yStride+1]-cptr[-yStride-1]))
              +square(int(cptr[ zStride-yStride]-cptr[-zStride-yStride]))];
          Ct=C[square(int(cptr[z2]-c0))
             +square(int(cptr[ zStride+yStride]-cptr[ zStride-yStride]))
              +square(int(cptr[ zStride+1]-cptr[ zStride-1]))];
          Cb=C[square(int(c0-cptr[-z2]))
              +square(int(cptr[-zStride+yStride]-cptr[-zStride-yStride]))
              +square(int(cptr[-zStride+1]-cptr[-zStride-1]))];
          Out[index2++] = c0 + (char)(
                            (Ce*int(cptr[2]-c0)
                            -Cw*int(c0-cptr[-2])
                          +Cn*int(cptr[2*yStride]-c0)
              -Cs*int(c0-cptr[-2*yStride])
              +Ct*int(cptr[z2]-c0)
              -Cb*int(c0-cptr[-z2])));
          cptr++;
        }
        cptr += 6;
        index2 += 6;
      }
    }
    index2 = vIn.index(3,3,Kmax);
    cptr = In + index2;
    for (int i=3; i<Imax; i++)
    {
      for (int j=3; j<Jmax; j++)
      {
        uint8 c0 = *cptr;
        Ce=C[(square(int(cptr[2]                           -c0))
           +square(int(cptr[yStride + 1]-cptr[- yStride + 1]))
            +square(int(cptr[zStride + 1]-cptr[- zStride + 1])))];
        Cw=C[(square(int(c0               -cptr[-2           ]))
           +square(int(cptr[yStride - 1]-cptr[-yStride - 1]) )
            +square(int(cptr[zStride - 1]-cptr[-zStride - 1]) ))];
        Cn=C[(square(int(cptr[2*yStride  ]-                 c0))
           +square(int(cptr[  yStride+1]-cptr[ yStride-1]))
            +square(int(cptr[ zStride+yStride]-cptr[-zStride+yStride])))];
        Cs=C[(square(int(c0-cptr[-2*yStride]))
           +square(int(cptr[-yStride+1]-cptr[-yStride-1]))
            +square(int(cptr[ zStride-yStride]-cptr[-zStride-yStride])))];
        Ct=C[(square(int(c0))
              +square(int(cptr[ zStride+yStride]-cptr[ zStride-yStride]))
            +square(int(cptr[ zStride+1]-cptr[ zStride-1])))];
        Cb=C[(square(int(c0-cptr[-z2]))
           +square(int(cptr[-zStride+yStride]-cptr[-zStride-yStride]))
            +square(int(cptr[-zStride+1]-cptr[-zStride-1])))];
        Out[index2++] = c0 + (char)(
                          (Ce*int(cptr[2]-c0)
                          -Cw*int(c0-cptr[-2])
                        +Cn*int(cptr[2*yStride]-c0)
            -Cs*int(c0-cptr[-2*yStride])
            +Ct*int(-c0)
            -Cb*int(c0-cptr[-z2])));
        cptr++;
      }
      cptr += 6;
      index2 += 6;
    }
    if (n!=(nIterations-1)) memcpy(In,Out,datasize+2*slicesize);
  }
  delete[] In;
  vOut.makeCompatible(vIn);
  memcpy(vOut.start(),Out+slicesize,datasize);
  delete[] Out;
  return true;
}
