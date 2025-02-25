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

#include "graph.h"

const int Graph::sentinel = 0;

int Graph::makemap(LabelT *map)
{
  int nLabels = 0;
  for (int i=0;i<nlists;i++) map[i] = sentinel;
  for (int i=0;i<nlists;i++)
  {
    if (map[i]==sentinel)
    {
      ++nLabels;
      visit(map, i, nLabels);
    }
  }
  return nLabels;
}

void Graph::visit(LabelT *map, int iNode, int labels)
{
  stack.push_back(iNode);
  while (stack.size()>0)
  {
    iNode = stack.back(); stack.pop_back();
    map[iNode] = labels;
    for (auto *node = lists[iNode]; node != 0; node = node->next)
    {
      if (map[node->data]==sentinel)
      {
        stack.push_back(node->data);
        map[node->data] = -1;
      }
    }
  }
}
