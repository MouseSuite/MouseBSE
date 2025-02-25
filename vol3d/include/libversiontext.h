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

#ifndef LibVersionText_H
#define LibVersionText_H
// used for Microsoft Visual C++ compiler to determine which libraries to link
#ifdef _WIN64
#define PLATFORM "x64"
#else
#define PLATFORM "Win32"
#endif

#ifdef _DLL
#define LIBTYPE "MD"
#else
#define LIBTYPE "MT"
#endif

#ifdef _DEBUG
#define DEBUG_TEXT "d"
#else
#define DEBUG_TEXT ""
#endif

#define LIBVERSIONTEXT "_" PLATFORM "_" LIBTYPE DEBUG_TEXT

#endif
