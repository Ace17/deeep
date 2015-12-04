/**
 * @brief Level 2
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cassert>
#include <random>
#include <chrono>
#include "engine/geom.h"
#include "engine/util.h"
#include "game/game.h"
#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/graph_tools.h"

const auto W = 32 * 2;
const auto H = 32 * 2;

const auto BLOCK_W = 8;
const auto BLOCK_H = 8;

int interpretTile(Vector2i pos, Vector2i& start, IGame* game, int val);

template<typename Gen>
vector<Edge> createRandomSpanningTree(Gen& gen, int cols, int rows)
{
  auto graph = createGridGraph(cols, rows);
  auto order = shuffle(seq(0, graph.size()), gen);

  vector<int> unused;
  auto tree = spanningTree(graph, order, unused);

  return extract(graph, tree);
}

struct Cell
{
  bool openLeft;
  bool openRight;
  bool openUp;
  bool openDown;
};

void connect(Matrix<Cell>& cells, Vector2i a, Vector2i b)
{
  assert(a.x == b.x || a.y == b.y);

  if(a.x > b.x)
    swap(a, b);

  if(a.y > b.y)
    swap(a, b);

  assert(a.x <= b.x);
  assert(a.y <= b.y);

  if(a.x + 1 == b.x)
  {
    assert(a.y == b.y);

    cells.get(a.x, a.y).openRight = true;
    cells.get(a.x + 1, a.y).openLeft = true;
  }
  else if(a.y + 1 == b.y)
  {
    assert(a.x == b.x);

    cells.get(a.x, a.y).openUp = true;
    cells.get(a.x, a.y + 1).openDown = true;
  }
}

Matrix<Cell> createConnectionMatrix(int numCols, int numRows, default_random_engine& gen)
{
  auto const tree = createRandomSpanningTree(gen, numCols, numRows);

  auto cells = Matrix<Cell>(Size2i(numCols, numRows));

  auto nodePos = [ = ] (int id) -> Vector2i
                 {
                   return Vector2i(id % numCols, id / numCols);
                 };

  for(auto edge : tree)
  {
    auto nodeA = nodePos(edge.a);
    auto nodeB = nodePos(edge.b);
    connect(cells, nodeA, nodeB);
  }

  return cells;
}

void loadLevel2(Matrix<int>& tiles, Vector2i& start, IGame* game)
{
  default_random_engine gen;

  auto seedVal = chrono::system_clock::now().time_since_epoch().count();
  gen.seed(seedVal);

  auto const numCols = ceilDiv(W, BLOCK_W);
  auto const numRows = ceilDiv(H, BLOCK_H);

  auto cells = createConnectionMatrix(numCols, numRows, gen);

  {
    auto emptyCell = [&] (int, int, int& tile) { tile = ' '; };
    tiles.scan(emptyCell);
  }

  auto rect = [&] (Vector2i pos, Size2i size, int val)
              {
                for(int y = 0; y < size.height; ++y)
                  for(int x = 0; x < size.width; ++x)
                    tiles.set(x + pos.x, y + pos.y, val);
              };

  for(int row = 0; row < numRows; ++row)
  {
    for(int col = 0; col < numCols; ++col)
    {
      auto& cell = cells.get(col, row);

      auto bRow = row * BLOCK_H;
      auto bCol = col * BLOCK_W;

      if(!cell.openDown)
        rect(Vector2i(bCol, bRow), Size2i(BLOCK_W, 3), 'X');

      if(!cell.openUp)
        rect(Vector2i(bCol, bRow + BLOCK_H - 2), Size2i(BLOCK_W, 2), 'X');

      if(!cell.openLeft)
        rect(Vector2i(bCol, bRow), Size2i(2, BLOCK_H), 'X');

      if(!cell.openRight)
        rect(Vector2i(bCol + BLOCK_W - 2, bRow), Size2i(2, BLOCK_H), 'X');
    }
  }

  tiles.get(4, 4) = '!';

  {
    auto onCell = [&] (int x, int y, int& tile)
                  {
                    tile = interpretTile(Vector2i(x, y), start, game, tile);
                  };

    tiles.scan(onCell);
  }
}

