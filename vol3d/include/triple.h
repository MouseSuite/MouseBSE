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

#ifndef Triple_H
#define Triple_H
#include <iostream>

template <class T>
class Triple {
public:
  Triple() {}
  Triple(T a_, T b_, T c_) : a(a_), b(b_), c(c_) {}
  T a;
  T b;
  T c;
};

template <class T>
inline std::ostream &operator<<(std::ostream & s, const Triple<T>& f)
{
  return s<<f.a<<'\t'<<f.b<<'\t'<<f.c;
}

template <class T>
inline std::istream &operator>>(std::istream & s, Triple<T>& f)
{
  return s>>f.a>>f.b>>f.c;
}

#endif
