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

#ifndef Codec32_H
#define Codec32_H

class Codec32 {
public:
  typedef unsigned char uint8;
  typedef unsigned int uint32;
  static void encode(const uint8 *data, uint32 *code, const int cx, const int cy, const int cz);
  static void decode(const uint32 *code, uint8 *data, const int cx, const int cy, const int cz);
};

#endif
