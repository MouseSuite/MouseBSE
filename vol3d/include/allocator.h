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

#ifndef DSAllocator_H
#define DSAllocator_H
#include <DS/memcheck.h>

template <class T>
class Allocator {
public:
  class TNode {
  public:
    TNode(T *n, TNode *p) : node(n), prev(p) {}
    T *node;
    TNode *prev;
  };
  Allocator(int blockSize_) : blockSize(blockSize_), nNodes(0), deadpool(0)
  {
    nodes = memcheck(new T[blockSize]);
  }
  T *newNode()
  {
    if (nNodes>=blockSize)	// memory pool is spent, allocate more.
    {
// First, save the current pool for deletion.
      deadpool = memcheck(new TNode(nodes,deadpool));
      nodes = memcheck(new T[blockSize]); // Allocate another block.
      if (nodes==0) return 0;
      nNodes = 0;
    }
    T *ret = nodes + nNodes;
    nNodes++;
    return ret;
  }
  void purge()
  {
    nNodes = 0;
    while (deadpool!=0)
    {
      TNode *prev = deadpool->prev;
      delete[] deadpool->node;
      delete deadpool;
      deadpool = prev;
    }
  }
  ~Allocator()
  {
    purge();
    delete[] nodes;
  }
  int nodecount() { return nNodes; }
private:
  const int blockSize;
  int	nNodes;
  T *nodes;
  TNode *deadpool;
};

#endif


