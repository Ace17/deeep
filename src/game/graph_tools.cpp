/**
 * @file graph_tools.cpp
 * @brief Graph utilies.
 * @author Sebastien Alaiwan
 * @date 2015-11-26
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "graph_tools.h"
#include <vector>
#include <map>

using namespace std;

int highestParent(map<int, int> const& parents, int element)
{
  auto i_parent = parents.find(element);

  if(i_parent == parents.end())
    return element;

  return highestParent(parents, i_parent->second);
};

/**
 * @brief computes a random spanning tree for graph 'edges'
 *
 * @return a list of kept edges.
 */
vector<int> spanningTree(vector<Edge> const& edges, vector<int> const& edgeOrder, vector<int>& discardedEdges)
{
  vector<int> r;

  map<int, int> parents;

  for(auto i : edgeOrder)
  {
    auto& edge = edges[i];

    auto const pa = highestParent(parents, edge.a);
    auto const pb = highestParent(parents, edge.b);

    // same connex component?
    if(pa == pb)
    {
      discardedEdges.push_back(i);
      continue;
    }

    parents[pb] = pa;

    r.push_back(i);
  }

  return r;
}

vector<Edge> createGridGraph(int cols, int rows)
{
  auto nodeId = [ = ] (int col, int row)
                {
                  return col + row * cols;
                };

  vector<Edge> r;

  for(int row = 0; row < rows; ++row)
  {
    for(int col = 0; col < cols; ++col)
    {
      auto const node = nodeId(col, row);

      if(row > 0)
      {
        auto const aboveNode = nodeId(col, row - 1);
        r.push_back(Edge(aboveNode, node));
      }

      if(col > 0)
      {
        auto const leftNode = nodeId(col - 1, row);
        r.push_back(Edge(leftNode, node));
      }
    }
  }

  return r;
}

