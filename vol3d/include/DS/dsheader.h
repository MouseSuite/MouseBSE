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

#ifndef DSHeader_H
#define DSHeader_H

#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

#define DT_NONE                 0 
#define DT_UNKNOWN              0      /*Unknown data type*/ 
#define DT_BINARY               1      /*Binary (1 bit per voxel)*/ 
#define DT_UNSIGNED_CHAR        2      /*Unsigned character (8 bits per voxel)*/ 
#define DT_SIGNED_SHORT         4      /*Signed short (16 bits per voxel)*/ 
#define DT_SIGNED_INT           8      /*Signed integer (32 bits per voxel)*/ 
#define DT_FLOAT                16     /*Floating point (32 bits per voxel)*/ 
#define DT_COMPLEX              32     /*Complex (64 bits per voxel; 2 floating point numbers) */
#define DT_DOUBLE               64     /*Double precision (64 bits per voxel)*/ 
#define DT_RGB                  128    /* */
#define DT_ALL                  255    /* */

                            /*------------------- new codes for NIFTI ---*/
#define DT_INT8                  256     /* signed char (8 bits)         */
#define DT_UINT16                512     /* unsigned short (16 bits)     */
#define DT_UINT32                768     /* unsigned int (32 bits)       */
#define DT_INT64                1024     /* long long (64 bits)          */
#define DT_UINT64               1280     /* unsigned long long (64 bits) */
#define DT_FLOAT128             1536     /* long double (128 bits)       */
#define DT_COMPLEX128           1792     /* double pair (128 bits)       */
#define DT_COMPLEX256           2048     /* long double pair (256 bits)  */

struct AnalyzeHeader {
  AnalyzeHeader()
  {
    memset((char *)this,0,sizeof(*this));
    regular = 'r';
    dims = 3;
    sizeof_hdr=sizeof(*this);
  }
// header_key
  int		sizeof_hdr;	/* For ANALYZE compatibility only */
  char		pad1[28];
  int		extents;	/* For ANALYZE compatibility only */
  char		pad2[2];
  char		regular;	/* For ANALYZE compatibility only */
  char		pad3;
// image_dimension
  short int	dims;		/* For ANALYZE compatibility only */
  short int	x_dim;		/* AIR */
  short int	y_dim;		/* AIR */
  short int	z_dim;		/* AIR */
  short int	t_dim;		/* For ANALYZE compatibility only */
  char		pad4[20];
  short int	datatype;	/* For ANALYZE compatibility only */
  short int	bits;		/* AIR */
  char		pad5[6];
  float		x_size;		/* AIR */
  float		y_size;		/* AIR */
  float		z_size;		/* AIR */
  char		pad6[48];
  int		glmax;		/* AIR */
  int		glmin;		/* AIR */
//	data_history
  char		descrip[80];	/* AIR (non-essential) */
  char		pad7[120];
};


void swapHeader(AnalyzeHeader &h);

inline void headername(std::string &dst, const char *charbuf)
{
  std::ostringstream ostr;
  std::string src(charbuf);
  size_t pos = src.rfind('.');
  std::string extension(".hdr");
  if (pos == src.npos)
  {
    ostr<<src.c_str()<<".hdr";
  }
  else
  {
    ostr<<src.substr(0,pos).c_str()<<extension.c_str();
  }
  dst = ostr.str();
}

inline bool readHeader(const char *ifname, AnalyzeHeader& header)
{
  std::string hfname;
  headername(hfname,ifname);
    std::ifstream ifile(hfname.c_str(),std::ios::binary);
  if (!ifile) return false;
  ifile.read((char *)&header,sizeof(AnalyzeHeader));
  return true;
}

inline bool writeHeader(const char *ofname, AnalyzeHeader& header)
{
  std::string hfname;
  headername(hfname,ofname);
    std::ofstream ofile(hfname.c_str(),std::ios::binary);
  if (!ofile) return false;
  ofile.write((char *)&header,sizeof(AnalyzeHeader));
  return true;
}

inline void byteswap(unsigned short &X)
{
  unsigned short a = (unsigned char)(X>>8);
  unsigned char b = (unsigned char)X;
  X = (int(b)<<8)|a;
}

inline void byteswap(short &X)
{
  unsigned char a = (unsigned char)(X>>8);
  unsigned char b = (unsigned char)X;
  X = (int(b)<<8)|a;
}

inline void swapx(unsigned int &X)
{
  unsigned short a = X>>16;
  byteswap(a);
  unsigned short b = (unsigned short)(X);
  byteswap(b);
  X = (int(b)<<16)|a;
}

inline void swapf(unsigned int *X)
{
  unsigned short a = *X>>16;
  byteswap(a);
  unsigned short b = (unsigned short)(*X);
  byteswap(b);
  *X = (int(b)<<16)|a;
}

inline void swapx(int &X)
{
  unsigned short a = X>>16;
  byteswap(a);
  unsigned short b = (unsigned short)(X);
  byteswap(b);
  X = (int(b)<<16)|a;
}

inline void swapHeader(AnalyzeHeader &h)
{
  swapx(h.sizeof_hdr); // int		sizeof_hdr;
  //char		pad1[28];
  swapx(h.extents);		//int		extents;	/* For ANALYZE compatibility only */
  //char		pad2[2];
  //char		regular;	/* For ANALYZE compatibility only */
  //char		pad3;
  byteswap(h.dims); //short int	dims;		/* For ANALYZE compatibility only */
  byteswap(h.x_dim);//short int	x_dim;		/* AIR */
  byteswap(h.y_dim);//short int	y_dim;		/* AIR */
  byteswap(h.z_dim);//short int	z_dim;		/* AIR */
  byteswap(h.t_dim);//short int	t_dim;		/* For ANALYZE compatibility only */
  //char		pad4[20];
  byteswap(h.datatype);//short int	datatype;	/* For ANALYZE compatibility only */
  byteswap(h.bits);//short int	bits;		/* AIR */
  //char		pad5[6];
  swapf((unsigned int *)&h.x_size);//float		x_size;		/* AIR */
  swapf((unsigned int *)&h.y_size);//float		y_size;		/* AIR */
  swapf((unsigned int *)&h.z_size);//float		z_size;		/* AIR */
//	swapf((unsigned int *)&h.x_orig);//float		x_size;		/* AIR */
//	swapf((unsigned int *)&h.y_orig);//float		y_size;		/* AIR */
//	swapf((unsigned int *)&h.z_orig);//float		z_size;		/* AIR */
  //char		pad6[48];
  swapx(h.glmax);//int		glmax;		/* AIR */
  swapx(h.glmin);//int		glmin;		/* AIR */
  //char		descrip[80];	/* AIR (non-essential) */
  //char		pad7[120];
};

#endif
