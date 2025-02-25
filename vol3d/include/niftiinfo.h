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

#ifndef NIFTIInfo_H
#define NIFTIInfo_H

#include <nifti1.h>

namespace SILT {
class NIFTIInfo {
public:
  NIFTIInfo(const nifti_1_header &hdr)
  {
    valid=true;
    qform_code = hdr.qform_code;
    sform_code = hdr.sform_code;
    quatern_b=hdr.quatern_b;
    quatern_c=hdr.quatern_c;
    quatern_d=hdr.quatern_d;
    qoffset_x=hdr.qoffset_x;
    qoffset_y=hdr.qoffset_y;
    qoffset_z=hdr.qoffset_z;
    for (int i=0;i<4;i++)
    {
      srow_x[i]=hdr.srow_x[i];
      srow_y[i]=hdr.srow_y[i];
      srow_z[i]=hdr.srow_z[i];
    }
  }
  NIFTIInfo() : valid(false), qform_code(0), sform_code(0),
    quatern_b(0), quatern_c(0), quatern_d(0),
    qoffset_x(0),	qoffset_y(0),	qoffset_z(0)
  {
    for (int i=0;i<4;i++)
    {
      srow_x[i]=0;
      srow_y[i]=0;
      srow_z[i]=0;
    }
  }
  bool valid;
  short qform_code ;   /*!< NIFTI_XFORM_* code.   */  /*-- all ANALYZE 7.5 ---*/
  short sform_code ;   /*!< NIFTI_XFORM_* code.   */  /*   fields below here  */
  /*   are replaced       */
  float quatern_b ;    /*!< Quaternion b param.   */
  float quatern_c ;    /*!< Quaternion c param.   */
  float quatern_d ;    /*!< Quaternion d param.   */
  float qoffset_x ;    /*!< Quaternion x shift.   */
  float qoffset_y ;    /*!< Quaternion y shift.   */
  float qoffset_z ;    /*!< Quaternion z shift.   */

  float srow_x[4] ;    /*!< 1st row affine transform.   */
  float srow_y[4] ;    /*!< 2nd row affine transform.   */
  float srow_z[4] ;    /*!< 3rd row affine transform.   */
};
}

#endif
