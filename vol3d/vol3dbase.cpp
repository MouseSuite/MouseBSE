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

#include <vol3dreorder.h>
#include <vol3dbase.h>

bool Vol3DBase::noRotate=false; // set to TRUE to block all auto-rotation

Vol3DBase::Vol3DBase() : cx(0), cy(0), cz(0), rx(1), ry(1), rz(1),
  fileOrientation(SILT::Mat3<float32>::Identity),
  currentOrientation(SILT::Mat3<float32>::Identity),
  transformCurrenttoFile(SILT::Mat3<sint32>::Identity)
{
}

Vol3DBase::~Vol3DBase()
{
}

std::string Vol3DBase::datatypeName() const
{
  return SILT::datatypeName(typeID());
}

bool Vol3DBase::setQForm(nifti_1_header &header) const
// set voxel dimensions and orientation
{
  header.pixdim[0] = 1; // assume qfac is one for now
  header.pixdim[1] = rx;
  header.pixdim[2] = ry;
  header.pixdim[3] = rz;
  // initialize qform with identity transform
  header.qform_code = 1;
  header.quatern_b = 0;    // Quaternion b param.
  header.quatern_c = 0;    // Quaternion c param.
  header.quatern_d = 0;    // Quaternion d param.
  {
    // we assume that R has correct unit scaling.
    double a=1,b=0,c=0,d=0; // identity quaternion values
    auto R=currentOrientation;
    if (std::abs(R.determinant()-1)>1e-2) std::cerr<<"the orientation matrix is not proper (i.e., |R|="<<R.determinant()<<"). Please check the output"<<std::endl;
    float qfac = ((R.determinant()-1) < 1e-2) ? 1 : -1;
    if (qfac < 0) { R.m02*=-1; R.m12*=-1; R.m22*=-1; } // flip last column for qfac = -1.
    a = R.m00 + R.m11 + R.m22 + 1;
    if (a>0.5)
    {
      a = 0.5 * std::sqrt(a);
      b = 0.25 * (R.m21-R.m12)/a;
      c = 0.25 * (R.m02-R.m20)/a;
      d = 0.25 * (R.m10-R.m01)/a;
    }
    else
    {
      double xd = 1 + R.m00 - (R.m11+R.m22);
      double yd = 1 + R.m11 - (R.m00+R.m22);
      double zd = 1 + R.m22 - (R.m00+R.m11);
      if (xd>0)
      {
        b = 0.5 * std::sqrt(xd) ;
        c = 0.25 * (R.m01+R.m10) /b;
        d = 0.25 * (R.m02+R.m20) /b;
        a = 0.25 * (R.m21-R.m12) /b;
      }
      else if (yd>0)
      {
        c = 0.5 * std::sqrt(yd) ;
        b = 0.25 * (R.m01+R.m10) /c;
        d = 0.25 * (R.m12+R.m21) /c;
        a = 0.25 * (R.m02-R.m20) /c;
      }
      else
      {
        d = 0.5 * std::sqrt(zd) ;
        b = 0.25 * (R.m02+R.m20) /d;
        c = 0.25 * (R.m12+R.m21) /d;
        a = 0.25 * (R.m10-R.m01) /d;
      }
      if (a<0)
      {
        b=-b;
        c=-c;
        d=-d;
        a=-a;
      }
    }
    if (c==0) c=0; // added to avoid -0, which results for some calculations of identity matrix
    header.quatern_b = b;
    header.quatern_c = c;
    header.quatern_d = d;
    header.qform_code = 1;// % NIFTI_XFORM_SCANNER_ANAT
  }
  header.qoffset_x = origin.x;    // Quaternion x shift.
  header.qoffset_y = origin.y;    // Quaternion y shift.
  header.qoffset_z = origin.z;    // Quaternion z shift.
  return true;
}

bool Vol3DBase::setSForm(nifti_1_header &header) const // set voxel dimensions and orientation
{
  SILT::Mat3<float32> Sm=currentOrientation*SILT::Mat3<float32>::diagonal(DSPoint(rx,ry,rz));
  header.sform_code=1; // NIFTI_XFORM_SCANNER_ANAT
  header.srow_x[0]=Sm.m00; header.srow_x[1]=Sm.m01; header.srow_x[2]=Sm.m02; header.srow_x[3]=origin.x;
  header.srow_y[0]=Sm.m10; header.srow_y[1]=Sm.m11; header.srow_y[2]=Sm.m12; header.srow_y[3]=origin.y;
  header.srow_z[0]=Sm.m20; header.srow_z[1]=Sm.m21; header.srow_z[2]=Sm.m22; header.srow_z[3]=origin.z;
  return true;
}

bool Vol3DBase::setHeader(nifti_1_header &header) const
{
  header.dim[0] = 3;
  header.dim[1] = cx;
  header.dim[2] = cy;
  header.dim[3] = cz;
  for (int i=4;i<8;i++) header.dim[4] = 1;
  header.pixdim[0] = 1.0f;
  header.pixdim[1] = rx;
  header.pixdim[2] = ry;
  header.pixdim[3] = rz;
  for (int i=4;i<8;i++) header.pixdim[i] = 1;
  header.glmin = 0;
  header.glmax = maxVal();
  header.datatype = niftiTypeID();
  header.bitpix = databits();
  size_t n = description.size();
  if (n<79)
  {
    std::copy(description.begin(),description.end(),header.descrip);
    header.descrip[description.length()] = 0;
  }
  else
  {
    for (int i=0;i<79;i++) header.descrip[i] = description.at(i);
    header.descrip[79]=0;
  }
  header.extents = 16384;
  header.bitpix = databits();
  header.glmin = minVal();
  header.glmax = maxVal();
  setSForm(header);
  setQForm(header);
  return true;
}

bool Vol3DBase::scanSForm(const nifti_1_header &header)
{
  if (header.sform_code>0)
  {
    DSPoint v0(header.srow_x[0],header.srow_y[0],header.srow_z[0]); // first column
    DSPoint v1(header.srow_x[1],header.srow_y[1],header.srow_z[1]); // second column
    DSPoint v2(header.srow_x[2],header.srow_y[2],header.srow_z[2]); // third column
    rx=v0.mag();
    ry=v1.mag();
    rz=v2.mag();
    fileOrientation = SILT::Mat3<float>::fromColumns(v0/rx,v1/ry,v2/rz);
    currentOrientation = fileOrientation;
    origin=DSPoint(header.srow_x[3],header.srow_y[3],header.srow_z[3]);
    return true;
  }
  std::cerr<<"sform not valid -- assuming identity matrix"<<std::endl;
  rx=header.pixdim[1];
  ry=header.pixdim[2];
  rz=header.pixdim[3];
  fileOrientation=currentOrientation=SILT::Mat3<float>(SILT::Mat3<float>::Identity);
  return false;
}

bool Vol3DBase::scanQForm(const nifti_1_header &header)
{
// see also http://nifti.nimh.nih.gov/pub/dist/src/niftilib/nifti1_io.c
  if (header.qform_code<=0) return false;
  origin=DSPoint(header.qoffset_x, header.qoffset_y, header.qoffset_z);

  double b=header.quatern_b;
  double c=header.quatern_c;
  double d=header.quatern_d;
  double a = 1.0l - (b*b + c*c + d*d) ;
  double qfac=(header.pixdim[0]<0) ? -1 : 1;
  if( a < 1.e-7l ) // test if a is really small...
  {                   /* special case */
    a = 1.0l / std::sqrt(b*b+c*c+d*d) ;
    b *= a ; c *= a ; d *= a ;        /* normalize (b,c,d) vector */
    a = 0.0l ;                        /* a = 0 ==> 180 degree rotation */
  }
  else
  {
    a = std::sqrt(a) ;                     /* angle = 2*arccos(a) */
  }
  fileOrientation=currentOrientation=SILT::Mat3<float>(
                                       a*a+b*b-c*c-d*d,    2*b*c-2*a*d,        qfac*(2*b*d+2*a*c),
                                       2*b*c+2*a*d,        a*a+c*c-b*b-d*d,    qfac*(2*c*d-2*a*b),
                                       2*b*d-2*a*c,        2*c*d+2*a*b,        qfac*(a*a+d*d-c*c-b*b)
                                       );
  rx=header.pixdim[1];
  ry=header.pixdim[2];
  rz=header.pixdim[3];
  return true;
}

bool Vol3DBase::makeCompatible(const Vol3DBase &vol)
{
  if (!setsize(vol.cx,vol.cy,vol.cz)) return false;
  rx = vol.rx;
  ry = vol.ry;
  rz = vol.rz;
  origin=vol.origin;
  description = vol.description;
  fileOrientation = vol.fileOrientation;
  currentOrientation = vol.currentOrientation;
  transformCurrenttoFile = vol.transformCurrenttoFile;
  niftiInfo = vol.niftiInfo;
  return true;
}

