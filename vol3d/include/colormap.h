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

#ifndef ColorMap_H
#define ColorMap_H

#include <map>
#include <vol3ddatatypes.h>
#include <dspoint.h>
#include <list>
#include <vector>

class ColorMap {
public:
  uint32 *operator()(std::string name)
  {
    MapType::iterator colormap = map.find(name);
    return (colormap != map.end()) ? colormap->second : (uint32 *) jetLUT;
  }
  ColorMap();
  typedef std::map<std::string,uint32 *> MapType;
  MapType map;
  bool loadCustomLUT(std::string lutFilename, bool verbose);
  static uint32 jetLUT[];
  static uint32 hsvLUT[];
  static uint32 coolLUT[];
  static uint32 copperLUT[];
  static uint32 pinkLUT[];
  static uint32 greyLUT[];
  static uint32 greyInverseLUT[];
  static uint32 hotLUT[];
  static uint32 redLUT[];
  static uint32 greenLUT[];
  static uint32 blueLUT[];
  static uint32 prismLUT[];
  static uint32 boneLUT[];
  static DSPoint rgb2pt(uint32 a)
  {
    unsigned char *p = (unsigned char *)&a;
    return DSPoint(p[2]/255.0f,p[1]/255.0f,p[0]/255.0f);
  }
  static DSPoint colormap(uint32 *LUT, float value, float lower, float upper)
  {
    float range = upper - lower;
    if (range==0) return rgb2pt(LUT[0]);
    int scale = (int)(255 * (value - lower)/range);
    if (scale<0) scale = 0;
    if (scale>255) scale = 255;
    return rgb2pt(LUT[scale]);
  }
  static void initialize(); // constructs grey inverse table
  std::list<std::vector<uint32>> customMaps;
};

#endif
