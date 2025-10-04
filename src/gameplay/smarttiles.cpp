/*
 * Copyright (C) 2021 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Auto-connecting tiles.

#include "base/geom.h"
#include "base/matrix.h"
#include "base/util.h"
#include <cassert>
#include <stdint.h>

typedef Matrix2<int> TileMap;

// .-------.
// | 5 4 3 |
// | 6 . 2 |
// | 7 0 1 |
// ._______.
static const Vec2i dirs[] =
{
  Vec2i(0, -1),
  Vec2i(+1, -1),
  Vec2i(+1, 0),
  Vec2i(+1, +1),
  Vec2i(0, +1),
  Vec2i(-1, +1),
  Vec2i(-1, 0),
  Vec2i(-1, -1),
};

static
bool hasNeighboor(TileMap const& occupancy, Vec2i myPos, Vec2i dir)
{
  auto const otherPos = myPos + dir;

  if(!occupancy.isInside(otherPos.x, otherPos.y))
    return true;

  auto const myTile = occupancy.get(myPos.x, myPos.y);
  auto const otherTile = occupancy.get(otherPos.x, otherPos.y);

  return otherTile == myTile;
}

static
int computeNeighboors(TileMap const& occupancy, int x, int y)
{
  int neighboors = 0;

  auto const c = Vec2i(x, y);

  uint32_t mask = 1;

  for(auto dir : dirs)
  {
    if(hasNeighboor(occupancy, c, dir))
      neighboors |= mask;

    mask <<= 1;
  }

  return neighboors;
}

// .-------.
// | 5 4 3 |
// | 6 . 2 |
// | 7 0 1 |
// ._______.
//
static
int computeTileComposition(int nb)
{
  switch(nb)
  {
  case 0b00000000: return 1;
  case 0b11111111: return 0;

  // ground
  case 0b11000111: return 4;
  case 0b11001111: return 4;
  case 0b11100111: return 4;

  // right wall
  case 0b00011111: return 13;
  case 0b10011111: return 13;
  case 0b00111111: return 13;

  // left wall
  case 0b11110001: return 5;
  case 0b11110011: return 5;
  case 0b11111011: return 5;

  // ceiling
  case 0b01111100: return 12;
  case 0b11111110: return 12;
  case 0b11111100: return 12;
  case 0b01111110: return 12;

  // outer corners
  case 0b00000111: return 2;
  case 0b00001111: return 2;
  case 0b10000111: return 2;

  case 0b11000001: return 3;
  case 0b11100001: return 3;
  case 0b11100011: return 3;
  case 0b11000011: return 3;

  // inner corners
  case 0b11111101: return 6;
  case 0b11110111: return 14;
  case 0b11011111: return 15;
  case 0b01111111: return 7;
  }

  return 1;
}

int computeTileFor(TileMap const& occupancy, int x, int y)
{
  auto const neighboors = computeNeighboors(occupancy, x, y);

  return computeTileComposition(neighboors);
}

