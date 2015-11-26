/**
 * @file graph_tools.h
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

#pragma once

#include <vector>

using namespace std;

struct Edge
{
  Edge(int a_, int b_) : a(a_), b(b_)
  {
  }

  int a, b;
};

/**
 * @brief computes a random spanning tree for graph 'edges'
 *
 * @return a list of kept edges.
 */
vector<int> spanningTree(vector<Edge> const& edges, vector<int> const& edgeOrder, vector<int>& discardedEdges);

vector<Edge> createGridGraph(int cols, int rows);

