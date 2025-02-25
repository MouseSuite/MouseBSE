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

#ifndef memcheck_H
#define memcheck_H

#include <string>
#include <iostream>
#include <sstream>

#define memcheck(p) memcheck_(p,__FILE__,__LINE__)

template<class T>
inline T* memcheck_(T* ptr, const char *file, const int line)
{
  if (ptr==0)
  {
    std::ostringstream ostr;
    ostr<<"Memory failure in "<<file<<" : "<<line<<".";
    std::cerr<<ostr.str()<<std::endl;
    throw ostr.str();
  }
  return ptr;
}

#endif
