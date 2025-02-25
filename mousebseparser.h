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

#ifndef MouseBSEParser_H
#define MouseBSEParser_H

#include <argparser.h>
#include <limits.h>

class MouseBSETool;

class MouseBSEParser : public ArgParserBase {
public:
  MouseBSEParser(MouseBSETool &bseTool);
  virtual bool validate()
  {
    if (mfname.empty() && ofname.empty() && erodedMaskFilename.empty() && hiresMask.empty() && cortexFilename.empty() && adfFilename.empty() && edgeFilename.empty())
    {
      std::cerr<<"error: no output files specified."<<std::endl;
      errcode=2;
      return false;
    }
    return true;
	}
	int closingSize;
  std::string ifname;
  std::string ofname;
  std::string mfname;
  std::string adfFilename;
  std::string initBrainFilename;
  std::string edgeFilename;
  std::string cortexFilename;
  std::string hiresMask;
  std::string erodedMaskFilename;
  std::string noneckFilename;
  int xMin=0,xMax=INT_MAX;
  int yMin=0,yMax=INT_MAX;
  int zMin=0,zMax=INT_MAX;
  int zpad=0;
};

#endif
