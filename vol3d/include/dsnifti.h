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

#ifndef DSNifti_H
#define DSNifti_H

#include <nifti1.h>
#include <silttypes.h>
#include <string.h>

class DSNifti : public nifti_1_header {
public:
  DSNifti()
  {
    sizeof_hdr=(sizeof(DSNifti));
    extents=16384;
    session_error=(0);
    regular=('r');
    dim_info=(0);
    std::fill(data_type,data_type+sizeof(data_type),0);
    std::fill(db_name,db_name+sizeof(db_name),0);
    for (int i=0;i<8;i++) dim[i]=0;
    intent_p1=0;
    intent_p2=0;
    intent_p3=0;
    intent_code=0;
    datatype=0;
    bitpix=0;
    slice_start=0;
    for (int i=0;i<8;i++) pixdim[i]=0;
    vox_offset=352;
    scl_slope=0;
    scl_inter=0;
    slice_end=0;
    slice_code=0;
    xyzt_units = NIFTI_UNITS_MM;
    cal_max=0;
    cal_min=0;
    slice_duration=0;
    toffset=0;
    glmax=0;
    glmin=0;
    std::fill_n(descrip,sizeof(descrip),0);
    std::fill_n(aux_file,sizeof(aux_file),0);
    qform_code=0;
    sform_code=0;
    quatern_b=0;
    quatern_c=0;
    quatern_d=0;
    qoffset_x=0;
    qoffset_y=0;
    qoffset_z=0;
    for (int i=0;i<4;i++) srow_x[i]=0;
    for (int i=0;i<4;i++) srow_y[i]=0;
    for (int i=0;i<4;i++) srow_z[i]=0;
    memset(intent_name,0,sizeof(intent_name));
// let's assume it's a single file.
    magic[0]='n';
    magic[1]='+';
    magic[2]='1';
    magic[3]='\0';
  }
  void setDatatype(SILT::DataType datatype);
};

#endif
