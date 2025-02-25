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

#ifndef CommonErrors_H
#define CommonErrors_H

class CommonErrors {
public:
  static int cantRead(std::string ifname) { std::cerr<<"error reading "<<ifname<<std::endl; return 1; }
  static int cantWrite(std::string ofname) { std::cerr<<"error writing "<<ofname<<std::endl; return 1; }
  static int incompatibleVolumes(std::string ofname, std::string ofname2) { std::cerr<<"error: "<<ofname<<" and "<<ofname2<<" have different dimensions."<<std::endl; return 1; }
};

#endif
