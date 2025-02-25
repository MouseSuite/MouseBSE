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

#ifndef RunLengthSegmenter_H
#define RunLengthSegmenter_H

#include <vol3d.h>
#include <vbit.h>
#include <vector>
#include <DS/runlength.h>
#include <DS/regioninfo.h>

class RunLengthSegmenter {
public:
  RunLengthSegmenter();
  ~RunLengthSegmenter();
  typedef int LabelType;
  enum Mode { D6 = 0, D18 = 1, D26 = 2 };
  static int intersect(RunLength& r1, RunLength& r2);
  static bool regionInfoGE(const RegionInfo &ri, const RegionInfo &ri2);
  int labelID(const int x, const int y, const int z); // find the ID of a given voxel, if it has one
  void setup(const int cx_, const int cy_, const int cz_);
  void label32FG(Vol3D<VBit> &imageOut) { label32FG(imageOut.raw32()); }
  void label32BG(Vol3D<VBit> &imageOut) { label32BG(imageOut.raw32()); }
  int regionCount(int n) const  
  {
    if (n<nregions)
      return regionInfo[n].count;
    return -1;
  }
  int nRegions() const { return nregions; }
  int segmentFG(Vol3D<VBit> &v)
  {
    setup(v.cx,v.cy,v.cz);
    return segment32FG(v.raw32(),v.raw32());
  }
  void segmentBG(Vol3D<VBit> &v)
  {
    setup(v.cx,v.cy,v.cz);
    segment32BG(v.raw32(),v.raw32());
  }
  std::vector<RegionInfo> regionInfo; // this should be behind an access function
  int CX() const { return cx; }
  int CY() const { return cy; }
  int CZ() const { return cz; }
  int presegmentBG(Vol3D<VBit> &v);
  int presegmentFG(Vol3D<VBit> &v);

  Mode mode;
  int cx;				// Image dimensions
  int cy;
  int cz;
protected:
  void remap(std::vector<LabelType> &newMap);
  int segmenttest32FG(uint8 *imageIn, uint32 *imageOut);
  int segmenttest32FG(uint32 *imageIn, uint8 *imageOut);
  void segment(uint8 *imageIn, uint8 *imageOut, uint8 zero, uint8 one);
  void label32FG(uint32 *imageOut);
  void label32BG(uint32 *imageOut);
protected:
  void population();
  int findRegion(const int cx, const int cy, const int cz);
  void findmax();
  void makeGraph();
  void makeGraph6();
  void makeGraph26();
  void makeGraph18();
  void label(uint8  *buffOut);
  void encode(uint8  *buffer);
  void encode32FG(uint32 *imageIn);
  void encode32BG(uint32 *imageIn);
  void segment32FG(uint8 *imageIn, uint32 *imageOut);
  int  segment32FG(uint32 *imageIn, uint32 *imageOut);
  void segment32BG(uint8 *imageIn, uint32 *imageOut);
  void segment32BG(uint32 *imageIn, uint32 *imageOut);

  std::vector<RunLength> runs;
  std::vector<int> linestart; // start of an x scan-line
  std::vector<LabelType> map,newmap;

  int nregions;
  uint8 high;
  uint8 low;
  int datasize;
  int runcount;
  int nsymbols;
  bool verbose;
  int NMax;
  int rlsPicked;
public:
  bool ensureCentered;
};

#endif
