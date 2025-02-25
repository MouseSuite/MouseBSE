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

#include <vol3d.h>
#include <vol3dquery.h>
#include <nifti1.h>
#include <endianswap.h>
#include <iomanip>
#include <cmath>
#include <DS/getfileinfo.h>

// quaternion code adapted from public domain code in http://nifti.nimh.nih.gov/pub/dist/src/niftilib/nifti1_io.c
void quaternionToMatrix(DSPoint &e1, DSPoint &e2, DSPoint &e3,
                        const float qb, const float qc, const float qd,
                        const float dx, const float dy, const float dz, const float qfac)
{
  /* last row is always [ 0 0 0 1 ] */
  /* compute a parameter from b,c,d */
  double b=qb,c=qc,d=qd;
  double a = 1.0l - (b*b + c*c + d*d) ;
  if (a < 1.e-7l)
  {                   /* special case */
    a = 1.0l / std::sqrt(b*b+c*c+d*d);
    b *= a ; c *= a ; d *= a ;        /* normalize (b,c,d) vector */
    a = 0.0l ;                        /* a = 0 ==> 180 degree rotation */
  }
  else
  {
    a = std::sqrt(a) ;                     /* angle = 2*arccos(a) */
  }
  /* load rotation matrix, including scaling factors for voxel sizes */
  double xd = (dx > 0.0) ? dx : 1.0l ;       /* make sure are positive */
  double yd = (dy > 0.0) ? dy : 1.0l ;
  double zd = (dz > 0.0) ? dz : 1.0l ;
  xd=yd=zd=1;
  if( qfac < 0.0 ) zd = -zd ;         /* left handedness? */

  e1.x =        (a*a+b*b-c*c-d*d) * xd ;
  e1.y = 2.0l * (b*c-a*d        ) * yd ;
  e1.z = 2.0l * (b*d+a*c        ) * zd ;
  e2.x = 2.0l * (b*c+a*d        ) * xd ;
  e2.y =        (a*a+c*c-b*b-d*d) * yd ;
  e2.z = 2.0l * (c*d-a*b        ) * zd ;
  e3.x = 2.0l * (b*d-a*c        ) * xd ;
  e3.y = 2.0l * (c*d+a*b        ) * yd ;
  e3.z =        (a*a+d*d-c*c-b*b) * zd ;
}

Vol3DQuery::Vol3DQuery() : headerType(HeaderType::NoHeader), datatype(SILT::Unknown),
  cx(0), cy(0), cz(0), rx(0), ry(0), rz(0), sx(0), sy(0), sz(0),
  filesize(-1), datastart(0), sizeOnDisk(-1),
  swapped(false), compressed(false),
  bitsPerVoxel(0)
{
}

void Vol3DQuery::swapNIFTIHeader(nifti_1_header &hdr)
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

bool Vol3DQuery::parseNIFTIheader(nifti_1_header hdr)
{
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
    quaternionToMatrix(e1,e2,e3,hdr.quatern_b,hdr.quatern_c,hdr.quatern_d,rx,ry,rz,hdr.pixdim[0]);
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
      // we do not handle these datatypes:
      // DT_INT64                1024     /* long long (64 bits)          */
      // DT_UINT64               1280     /* unsigned long long (64 bits) */
      // DT_FLOAT128             1536     /* long double (128 bits)       */
      // DT_COMPLEX128           1792     /* double pair (128 bits)       */
      // DT_COMPLEX256           2048     /* long double pair (256 bits)  */
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

bool Vol3DQuery::parseNIFTI(std::string ifname)
{
  filesize = getFileSize(ifname);
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
  parseNIFTIheader(hdr);
  return true;
}

bool Vol3DQuery::query(std::string ifname)
{
  datastart = 0;
  if (!findFile(ifname))
  {
    return false;
  }
  filename = ifname;
  headerType = HeaderType::NoHeader;
  if (StrUtil::hasExtension(StrUtil::gzStrip(ifname),".nii"))
  {
    parseNIFTI(ifname);
  }
  else if (StrUtil::hasExtension(ifname,".hdr"))
  {
    std::string basename = StrUtil::extStrip(ifname);
    std::string firstTry = basename + ".img";
    if (!isFile(firstTry))
    {
      firstTry += ".gz";
      if (!isFile(firstTry))
      {
        firstTry = basename;
        if (!isFile(firstTry))
        {
          firstTry += ".gz";
          if (!isFile(firstTry))
          {
            std::cout<<"Unable to find matching image file for "<<ifname<<std::endl;
            return false;
          }
        }
      }
    }
    {
      filename = firstTry;
      headerFilename = ifname;
      headerType = HeaderType::Analyze;// still need to query
    }
  }
  else
  {
    filename = ifname;
    std::string uncompressedname = StrUtil::gzStrip(ifname);
    std::string ext = StrUtil::getExt(uncompressedname);
    {
      std::string basename = StrUtil::extStrip(uncompressedname);
      std::string hdrname = basename + ".hdr";
      if (isFile(hdrname))
      {
        headerType = HeaderType::Analyze;
        headerFilename = hdrname;
      }
      else
      {
        headerType = HeaderType::NoHeader;
        std::ifstream ifile(ifname.c_str(),std::ios::binary);
        char buf[5] = { 0,0,0,0,0 };
        if (ifile.seekg(128))          // check for DICOM
        {
          ifile.read(buf,4);
          if ((buf[0]=='D')&&buf[1]=='I' && buf[2]=='C' && buf[3]=='M')
          {
            headerType = HeaderType::DICOM;
          }
        }
      }
    }
  }
  switch (headerType)
  {
    case HeaderType::Analyze:
      if (!parseAnalyze(filename)) return false;
      break;
    case HeaderType::NIFTI: // can't reach here (see above)
      {
      }
      break;
    case HeaderType::DICOM :
    {
      std::cerr<<"DICOM files are not currently supported in this version of the software."<<std::endl;
      return false;
    }
    default:
    {
      std::cerr<<"No file information available for file "<<ifname<<std::endl;
      return false;
    }
  }
  sizeOnDisk = getFileSize(ifname.c_str());
  compressed = (StrUtil::hasExtension(ifname,".gz"));
  filesize = (compressed) ? getGZipFilesize(ifname.c_str()) : sizeOnDisk;
  return true;
}

bool Vol3DQuery::parseAnalyze(std::string /*fname*/) // really only use headerFilename
{
  AnalyzeHeader hdr;
  hdr.regular = 'r';
  readHeader(headerFilename.c_str(),hdr);
// parse .hdr files as NIFTI if they have the magic word set 
  {
    nifti_1_header niftiHeader=*(nifti_1_header *)(&hdr);
    if (niftiHeader.magic[0]=='n' && niftiHeader.magic[1]=='i' && niftiHeader.magic[2]=='1' && niftiHeader.magic[3]==0)
    {
      parseNIFTIheader(niftiHeader);
      headerType = HeaderType::NIFTI_TWO_FILE;
      return true;
    }
  }
  description = std::string(hdr.descrip,80);
  if (hdr.sizeof_hdr!=348)
  {
    swapHeader(hdr);
    if (hdr.sizeof_hdr!=348)
    {
      std::cerr<<"Header is not in analyze format."<<std::endl;
      return false;
    }
    swapped = true;
  }
  else
    swapped = false;
  cx=hdr.x_dim;
  cy=hdr.y_dim;
  cz=hdr.z_dim;
  rx=hdr.x_size;
  ry=hdr.y_size;
  rz=hdr.z_size;
  bitsPerVoxel=hdr.bits;
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
    default               : datatype = SILT::Unknown; break;
  }
  if (hdr.datatype==DT_UNKNOWN)
  {
    if (hdr.bits==16)
    {
      datatype = (hdr.glmax>32767) ? SILT::Uint16 : SILT::Sint16;
    }
    else if (hdr.bits==8)
    {
      datatype = (hdr.glmin<0) ? SILT::Sint8 : SILT::Uint8;
    }
    else if (hdr.bits==sizeof(DSPoint)*8)
    {
      datatype = SILT::Vector3F;
    }
    else if (hdr.bits==sizeof(EigenSystem3x3f)*8)
    {
      datatype = SILT::Eigensystem3x3f;
    }
    else
    {
      std::cout<<"Unrecognized Analyze datatype with "<<hdr.bits<<" bits."<<std::endl;
    }
  }
  return true;
}

bool Vol3DQuery::findFile(std::string &queryname)
{
  if (isFile(queryname)) return true;
  if (StrUtil::isGZ(queryname)) return false;
  std::string searchOrder[] =
  {
    ".gz",
    ".nii",
    ".nii.gz",
    ".img",
    ".img.gz"
  };
  const int nQuery = sizeof(searchOrder)/sizeof(std::string);
  for (int i=0;i<nQuery;i++)
  {
    std::string query(queryname);
    query += searchOrder[i];
    if (isFile(query))
    {
      queryname = query;
      return true;
    }
  }
  return false;
}
