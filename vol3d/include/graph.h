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

#ifndef VOL3D_Graph_H
#define VOL3D_Graph_H

#include <vol3ddatatypes.h>
#include <allocator.h>
#include <vector>

class Graph {
public:
  class GraphNode {
  public:
    int data;
    GraphNode* next;
  };
  typedef GraphNode *GraphNodePtr;
  typedef int LabelT;
  Graph(int blocksize) : allocator(blocksize)
  {
    stack.reserve(1000000); // stacksize should be suitable for standard MRI -- was not being used as a parameter
  }
  ~Graph()
  {
  }
  void reset(int n)
  {
    allocator.purge();
    nlists = n;
    lists.resize(nlists);
    tails.resize(nlists);
    for (int i=0;i<nlists;i++)
    {
      tails[i] = nullptr;
      lists[i] = nullptr;
    }
  }
  void link(int a, int b);
  int makemap(LabelT *map);
private:
  void visit(LabelT *map, int iNode, int label);
  static const int sentinel;
  std::vector<GraphNode *> lists;
  std::vector<GraphNode *> tails;
  Allocator<GraphNode> allocator;
  int nlists{0};
  std::vector<int> stack;
};

inline void Graph::link(int a, int b)
{
  GraphNode *node = allocator.newNode();
  if (node==0)
  {
    std::cerr<<"Error ("<<__FILE__<<":"<<__LINE__<<") : unable to allocate memory for GraphNodes"<<std::endl;
    return;
  }
  node->data = b;
  node->next = 0;
  if (lists[a]==0)
  {
    lists[a] = node;
    tails[a] = lists[a];
  }
  else
  {
    tails[a]->next = node;
    tails[a] = tails[a]->next;
  }
}

#endif

































