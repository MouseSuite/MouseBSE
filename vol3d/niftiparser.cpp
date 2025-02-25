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

#include <niftiparser.h>
#include <endianswap.h>
#include <DS/getfileinfo.h>
#include <zstream.h>
#include <eigensystem3x3.h>

void quaternionToMatrix(DSPoint &e1, DSPoint &e2, DSPoint &e3,
                        const float qb, const float qc, const float qd,
                        const float dx, const float dy, const float dz, const float qfac);

void NIFTIParser::swapNIFTIHeader(nifti_1_header &hdr)
{
  SILT::endian_swap(hdr.sizeof_hdr);
  for (int i=0;i<8;i++)
    SILT::endian_swap(hdr.dim[i]);
  SILT::endian_swap(hdr.datatype);
  SILT::endian_swap(hdr.bitpix);
  SILT::endian_swap(hdr.slice_start);
  for (int i=0;i<8;i++)
    SILT::endian_swap(hdr.pixdim[i]);
  SILT::endian_swap(hdr.vox_offset);
  SILT::endian_swap(hdr.scl_slope);
  SILT::endian_swap(hdr.scl_inter);
  SILT::endian_swap(hdr.slice_end);
  SILT::endian_swap(hdr.cal_max);
  SILT::endian_swap(hdr.cal_min);
  SILT::endian_swap(hdr.slice_duration);
  SILT::endian_swap(hdr.toffset);
  SILT::endian_swap(hdr.glmax);
  SILT::endian_swap(hdr.glmin);
  SILT::endian_swap(hdr.qform_code);
  SILT::endian_swap(hdr.sform_code);
  SILT::endian_swap(hdr.quatern_b);
  SILT::endian_swap(hdr.quatern_c);
  SILT::endian_swap(hdr.quatern_d);
  SILT::endian_swap(hdr.qoffset_x);
  SILT::endian_swap(hdr.qoffset_y);
  SILT::endian_swap(hdr.qoffset_z);
  for (int i=0;i<4;i++)
    SILT::endian_swap(hdr.srow_x[i]);
  for (int i=0;i<4;i++)
    SILT::endian_swap(hdr.srow_y[i]);
  for (int i=0;i<4;i++)
    SILT::endian_swap(hdr.srow_z[i]);
}

bool NIFTIParser::parseNIFTI(std::string ifname)
{

  sizeOnDisk = getFileSize(ifname);
  SILT::izstream ifile(ifname);
  if (!ifile)
  {
    std::cerr<<"couldn't open "<<ifname<<std::endl;
    return false;
  }
  nifti_1_header hdr;
  ifile.read((char *)&hdr,sizeof(hdr));
  filename = ifname;
  headerFilename = ifname;
  headerType = HeaderType::NIFTI;// still need to query
  int nd = hdr.dim[0];
  if ((nd<0) || (nd>7))
  {
    swapNIFTIHeader(hdr);
    swapped = true;
  }
  else
    swapped = false;
  datastart = (int)hdr.vox_offset;
  description = std::string(hdr.descrip,80);
  cx=hdr.dim[1];//x_dim;
  cy=hdr.dim[2];//y_dim;
  cz=hdr.dim[3];//z_dim;
  ct=hdr.dim[4];//time;
  cv=hdr.dim[5];//vector;
  rx=hdr.pixdim[1];//x_size;
  ry=hdr.pixdim[2];//y_size;
  rz=hdr.pixdim[3];//z_size;
  if (hdr.sform_code>0)
  {
    e1 = DSPoint(hdr.srow_x[0],hdr.srow_y[0],hdr.srow_z[0]);
    e2 = DSPoint(hdr.srow_x[1],hdr.srow_y[1],hdr.srow_z[1]);
    e3 = DSPoint(hdr.srow_x[2],hdr.srow_y[2],hdr.srow_z[2]);
    float e1mag = e1.mag();
    if (e1mag>0)
    {
      e1 = e1/e1mag;
      rx=e1mag;
    }
    else
    {
      rx=1.0f; e1 = DSPoint(1,0,0);
      std::cerr<<"warning: sform in file "<<filename<<" has a zero eigenvalue"<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[0],hdr.srow_y[0],hdr.srow_z[0])<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[1],hdr.srow_y[1],hdr.srow_z[1])<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[2],hdr.srow_y[2],hdr.srow_z[2])<<std::endl;
    }
    float e2mag = e2.mag();
    if (e2mag>0)
    {
      e2 = e2/e2mag;
      ry=e2mag;
    }
    else
    {
      ry=1.0f;
      e2 = DSPoint(0,1,0);
      std::cerr<<"warning: sform in file "<<filename<<" has a zero eigenvalue"<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[0],hdr.srow_y[0],hdr.srow_z[0])<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[1],hdr.srow_y[1],hdr.srow_z[1])<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[2],hdr.srow_y[2],hdr.srow_z[2])<<std::endl;
    }
    float e3mag = e3.mag();
    if (e3mag>0)
    {
      e3 = e3/e3mag;
      rz=e3mag;
    }
    else
    {
      rz=1.0f;
      e3 = DSPoint(0,0,1);
      std::cerr<<"warning: sform in file "<<filename<<" has a zero eigenvalue"<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[0],hdr.srow_y[0],hdr.srow_z[0])<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[1],hdr.srow_y[1],hdr.srow_z[1])<<std::endl;
      std::cerr<<DSPoint(hdr.srow_x[2],hdr.srow_y[2],hdr.srow_z[2])<<std::endl;
    }
  }
  else if (hdr.qform_code!=0)
  {
    quaternionToMatrix(e1,e2,e3,hdr.quatern_b,hdr.quatern_c,hdr.quatern_d,
                       rx,ry,rz,
                       hdr.pixdim[0]);
    DSPoint offset(hdr.qoffset_x,hdr.qoffset_y,hdr.qoffset_z);
    origin = offset;
  }
  else
  {
    e1 = DSPoint(1,0,0);
    e2 = DSPoint(0,1,0);
    e3 = DSPoint(0,0,1);
  }
  bitsPerVoxel=hdr.bitpix;
  switch (hdr.datatype)
  {
    case DT_UNKNOWN       : datatype = SILT::Unknown; break;
    case DT_BINARY        : datatype = SILT::Unknown; break;
    case DT_UNSIGNED_CHAR : datatype = SILT::Uint8;   break;
    case DT_SIGNED_SHORT  : datatype = (hdr.glmax>32767) ? SILT::Uint16 : SILT::Sint16;  break;
    case DT_SIGNED_INT    : datatype = SILT::Sint32;  break;
    case DT_FLOAT         : datatype = SILT::Float32; break;
    case DT_COMPLEX       : datatype = SILT::Unknown; break;
    case DT_DOUBLE        : datatype = SILT::Float64; break;
    case DT_RGB           : datatype = SILT::RGB8;    break;
    case DT_INT8					: datatype = SILT::Sint8;		break; /* signed char (8 bits)         */
    case DT_UINT16				: datatype = SILT::Uint16;  break; /* unsigned short (16 bits)     */
    case DT_UINT32				: datatype = SILT::Uint32;  break; /* unsigned int (32 bits)       */
      //case DT_INT64                1024     /* long long (64 bits)          */
      //case DT_UINT64               1280     /* unsigned long long (64 bits) */
      //case DT_FLOAT128             1536     /* long double (128 bits)       */
      //case DT_COMPLEX128           1792     /* double pair (128 bits)       */
      //case DT_COMPLEX256           2048     /* long double pair (256 bits)  */
    default               : datatype = SILT::Unknown; break;
  }
  if (hdr.datatype==DT_UNKNOWN)
  {
    if (hdr.bitpix==16)
    {
      datatype = (hdr.glmax>32767) ? SILT::Uint16 : SILT::Sint16;
    }
    else if (hdr.bitpix==8)
    {
      datatype = (hdr.glmin<0) ? SILT::Sint8 : SILT::Uint8;
    }
    else if (hdr.bitpix==sizeof(DSPoint)*8)
    {
      datatype = SILT::Vector3F;
    }
    else if (hdr.bitpix==sizeof(EigenSystem3x3f)*8)
    {
      datatype = SILT::Eigensystem3x3f;
    }
    else
    {
      std::cerr<<"Unrecognized NIFTI datatype with "<<hdr.bitpix<<" bits."<<std::endl;
    }
  }
  niftiInfo = SILT::NIFTIInfo(hdr);
  return true;
}
