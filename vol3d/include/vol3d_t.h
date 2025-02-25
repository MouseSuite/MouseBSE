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

#ifndef Vol3D_T_H
#define Vol3D_T_H 

#include <cmath>
#include <vol3d.h>
#include <vol3dquery.h>
#include <zstream.h>
#include <endianswap.h>
#include <dsnifti.h>
#include <siltbyteswap.h>
#include <vol3dreorder.h>

template <class T>
size_t Vol3D<T>::readDataStream(SILT::izstream &ifile)
{
  size_t bytesReadTotal = 0;
  size_t bytesInImage=size()*sizeof(T);
  unsigned chunkSize = 1024*1024*1024; // matches type used in ifile.read
  size_t bytesRemaining=bytesInImage;
  char *dst=reinterpret_cast<char *>(&data[0]);
  while (bytesRemaining>0)
  {
    size_t bytesToRead=chunkSize<bytesRemaining ? chunkSize : bytesRemaining;
    auto bytesRead=ifile.read(dst,bytesToRead);
    if (bytesRead<=0) { std::cerr<<"error reading file: read incorrect number of bytes"<<std::endl; break; }
    dst += bytesRead;
    bytesReadTotal += static_cast<decltype(bytesReadTotal)>(bytesRead);
    bytesRemaining -= static_cast<decltype(bytesRemaining)>(bytesRead);
    if (bytesRead != static_cast<decltype(bytesRead)>(bytesToRead)) break;
  }
  if (bytesReadTotal != size()*sizeof(T))
  {
    std::cerr<<"warning: expected to read "<<size()*sizeof(T)<<", read "<<bytesReadTotal<<" bytes."<<std::endl;
  }
  return bytesReadTotal;
}

template <class T>
bool Vol3D<T>::readNifti(std::string ifname, Vol3DBase::AutoRotateCode autoRotate)
{
  SILT::izstream ifile(ifname);
  if (!ifile) return false;
  nifti_1_header header;
  ifile.read(reinterpret_cast<void *>(&header),sizeof(header));
  bool swapped=false;
  if ((header.dim[0]<0) || (header.dim[0]>7))
  {
    swapped=true;
    Vol3DQuery::swapNIFTIHeader(header);
  }
  if (!setsize(header.dim[1],header.dim[2],header.dim[3]))
  {
    std::cerr<<"Unable to allocate memory for new image.\n"<<std::endl;
    return false;
  }
  scl_slope=header.scl_slope;
  scl_inter=header.scl_inter;
  ifile.seekg(static_cast<off_t>(header.vox_offset),std::ios::beg); // TODO: should really test if header.vox_offset is valid
  readDataStream(ifile);
  if (swapped)
  {
    SILT::byteswap(&data[0],size()); // TODO: change to vector input
  }
  if (scanQForm(header))
  {
// read image dimensions/coordinates from q-form -- no need to read from s-form
  }
  else
  {
    if (scanSForm(header))
    {
    }
    else
      std::cerr<<"couldn't read coordinate system -- assuming analyze"<<std::endl;
  }
  if ((autoRotate==Vol3DBase::RotateToRAS)&&!Vol3DBase::noRotate)
  {
    if (Vol3DReorder::isCanonical(*this)==false)
    {
      Vol3DReorder::transformNIItoRAS(*this);
    }
  }
  return true;
}

template <class T>
bool Vol3D<T>::read(const Vol3DQuery &vq, Vol3DBase::AutoRotateCode autoRotate)
{
  filename = vq.filename;
  switch (vq.headerType)
  {
    case HeaderType::Analyze:
    {
      filename = vq.filename;
      SILT::izstream ifile(vq.filename.c_str());
      if (!ifile)
      {
        std::cerr<<"unable to read "<<vq.filename<<std::endl;
        return false;
      }
      if (!setsize(vq.cx,vq.cy,vq.cz))
      {
        std::cerr<<"Unable to allocate memory "<<std::endl;
        return false;
      }
      rx = vq.rx;
      ry = vq.ry;
      rz = vq.rz;
      readDataStream(ifile); // need to check read size
      if (vq.swapped) SILT::byteswap(&data[0],size());
      if (rx<0) { Vol3DReorder::flipX(*this); rx=-rx; }
      if (ry<0) { Vol3DReorder::flipY(*this); ry=-ry; }
      if (rz<0) { Vol3DReorder::flipZ(*this); rz=-rz; }
      return true;
      break;
    }
    case HeaderType::NIFTI_TWO_FILE:
      {
        SILT::izstream headerFile(vq.headerFilename);
        nifti_1_header header;
        headerFile.read(reinterpret_cast<void *>(&header),sizeof(header));
        bool swapped=false;
        if ((header.dim[0]<0) || (header.dim[0]>7))
        {
          swapped=true;
          Vol3DQuery::swapNIFTIHeader(header);
        }
        if (!setsize(header.dim[1],header.dim[2],header.dim[3]))
        {
          std::cerr<<"Unable to allocate memory for new image.\n"<<std::endl;
          return false;
        }
        filename = vq.filename;
        SILT::izstream ifile(vq.filename);
        if (!setsize(vq.cx,vq.cy,vq.cz))
        {
          std::cerr<<"Unable to allocate memory "<<std::endl;
          return false;
        }
        rx = vq.rx;
        ry = vq.ry;
        rz = vq.rz;
        readDataStream(ifile);
        if (swapped)
        {
          SILT::byteswap(&data[0],size());
        }
        if (scanQForm(header))
        {
      //read image dimensions/coordinates from q-form -- no need to read from s-form
        }
        else
        {
          if (scanSForm(header))
          {
          }
          else
            std::cerr<<"couldn't read coordinate system -- assuming analyze"<<std::endl;
        }
        if ((autoRotate==Vol3DBase::RotateToRAS)&&!Vol3DBase::noRotate)
        {
          if (Vol3DReorder::isCanonical(*this)==false)
          {
            Vol3DReorder::transformNIItoRAS(*this);
          }
        }
      }
      break;
    case HeaderType::NIFTI:
      return readNifti(filename,autoRotate);
      break;
    default:
      std::cerr<<"Unknown format for "<<filename<<std::endl;
      return false;
      break;
  }
  return true;
}

template <class T>
bool Vol3D<T>::read(std::string ifname, Vol3DBase::AutoRotateCode autoRotate)
{
  filename = ifname;
  Vol3DQuery vq;
  if (!vq.query(ifname))
  {
    return false;
  }
  return read(vq,autoRotate);
}

template <class T>
bool Vol3D<T>::write(std::string ofname)
{
  bool isNIFTI = StrUtil::hasExtension(StrUtil::gzStrip(ofname),".nii");
  bool isAnalyze = StrUtil::hasExtension(StrUtil::gzStrip(ofname),".img")||StrUtil::hasExtension(ofname,".hdr");
  if (!(isNIFTI||isAnalyze)) { ofname += ".nii.gz"; isNIFTI=true; }
  bool compress = StrUtil::isGZ(ofname);
  if (isNIFTI)
  {
    DSNifti hdr;
    setHeader(hdr);
    if (compress)
    {
      SILT::ozstream ofile(ofname.c_str());
      if (!ofile) return false;
      ofile.write(static_cast<void *>(&hdr), sizeof(hdr));
      char buf[4]={0,0,0,0};
      ofile.write(buf,4);
      {
        size_t bytesInImage=cx*cy*cz*sizeof(T);
        size_t chunkSize = 1024*1024*1024;
        size_t bytesRemaining=bytesInImage;
        char *src=reinterpret_cast<char *>(&data[0]);
        while (bytesRemaining>0)
        {
          size_t bytesToWrite=chunkSize<bytesRemaining ? chunkSize : bytesRemaining;
          auto bytesWritten=ofile.write(src, bytesToWrite);
          src += bytesWritten;
          if (bytesWritten<=0) { std::cerr<<"error saving "<<ofname<<std::endl; break; }
          bytesRemaining -= static_cast<decltype(bytesRemaining)>(bytesWritten); // bytesWritten should never be negative
          if (bytesWritten != static_cast<decltype(bytesWritten)>(bytesToWrite)) { std::cerr<<"error saving "<<ofname<<": wrote incorrect number of bytes "<<std::endl; break; }
        }
      }
    }
    else
    {
      std::ofstream ofile(ofname.c_str(),std::ios::binary);
      if (!ofile) return false;
      ofile.write(reinterpret_cast<char *>(&hdr), sizeof(hdr));
      char buf[4]={0,0,0,0};
      ofile.write(buf,4);
      ofile.write(reinterpret_cast<char *>(&data[0]), cx*cy*cz*sizeof(T));
    }
  }
  else
  {
    std::string headerFilename;
    if (StrUtil::hasExtension(ofname,".hdr"))
    {
      headerFilename=ofname;
      ofname = StrUtil::extStrip(StrUtil::gzStrip(headerFilename),"hdr")+".img.gz";
    }
    else
    {
      headerFilename = StrUtil::extStrip(StrUtil::gzStrip(ofname),"img") + ".hdr";
    }
    DSNifti niftiHeader;
    setHeader(niftiHeader);
    niftiHeader.magic[0]='n';
    niftiHeader.magic[1]='i';
    niftiHeader.magic[2]='i';
    niftiHeader.magic[3]='\0';
    if (compress)
    {
      SILT::ozstream ofile(ofname);
      if (!ofile) return false;
      ofile.write(static_cast<void *>(&data[0]), cx*cy*cz*sizeof(T));
    }
    else
    {
      std::ofstream ofile(ofname,std::ios::binary);
      if (!ofile) return false;
      ofile.write(reinterpret_cast<char *>(&data[0]), cx*cy*cz*sizeof(T));
    }
    std::ofstream hfile(headerFilename,std::ios::binary);
    if (!hfile) return false;
    hfile.write(reinterpret_cast<char *>(&niftiHeader),sizeof(DSNifti));
  }
  return true;
}

template <class T>
bool Vol3D<T>::maskWith(const Vol3D<uint8> &vMask)
{
  if (isCompatible(vMask)==false) return false;
  const int ds = size();
  T *dst = start();
  uint8 const *m = vMask.start();
  for (int i=0;i<ds;i++) if (!m[i]) dst[i] = 0;
  return true;
}

#define Vol3DInstance(T)\
  template bool Vol3D<T>::read(std::string, Vol3DBase::AutoRotateCode);\
  template bool Vol3D<T>::read(const Vol3DQuery &, Vol3DBase::AutoRotateCode);\
  template bool Vol3D<T>::write(std::string);\
  template bool Vol3D<T>::copyCast(std::unique_ptr<Vol3DBase> &) const; \
  template bool Vol3D<T>::maskWith(const Vol3D<uint8> &);\
  template bool Vol3D<T>::readNifti(std::string, Vol3DBase::AutoRotateCode);

template <class T>
bool Vol3D<T>::copyCast(std::unique_ptr<Vol3DBase> &dst) const
{
  if (!dst||this->typeID()!=dst->typeID()) // types don't match so we need to create a matching object
  {
    auto newVol = std::make_unique<Vol3D<T>>();
    if (!newVol) return false;
    newVol->copy(*this);
    dst = std::move(newVol);
  }
  else // we know it's the same type, so we can cast and do a direct copy
  {
    static_cast<Vol3D<T> *>(dst.get())->copy(*this);
  }
  return true;
}

#endif
