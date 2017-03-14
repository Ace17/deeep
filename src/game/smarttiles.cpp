/**
 * Auto-connecting tiles.
 */

/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */
#include <cassert>
#include <array>
#include "base/util.h"
#include "base/geom.h"

using namespace std;

typedef Matrix<int> TileMap;

static const Vector2i dirs[] =
{
  Vector2i(-1, -1),
  Vector2i(0, -1),
  Vector2i(+1, -1),
  Vector2i(+1, 0),
  Vector2i(+1, +1),
  Vector2i(0, +1),
  Vector2i(-1, +1),
  Vector2i(-1, 0),
};

static
bool hasNeighboor(TileMap const& occupancy, Vector2i myPos, Vector2i dir)
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

  auto const c = Vector2i(x, y);

  for(int i = 0; i < 8; ++i)
  {
    if(hasNeighboor(occupancy, c, dirs[i]))
      neighboors |= (1 << i);
  }

  return neighboors;
}

enum
{
  LB, // left bottom
  MB, // bottom
  RB, // right bottom
  RM, // right
  RT, // right top
  MT, // top
  LT, // left top
  LM, // left
};

enum TILECLASS
{
  TILECLASS_CORNER,
  TILECLASS_VERT_WALL,
  TILECLASS_HORZ_WALL,
  TILECLASS_INNER_CORNER,
  TILECLASS_PLAIN,
};

static
TILECLASS classify(int horz, int vert, int nb)
{
  // find the diagonal between horz and vert
  auto const diag = (horz + 1) % 8 == (vert + 8 - 1) % 8 ? (horz + 1) % 8 : (horz + 8 - 1) % 8;

  auto const horzMask = (1 << horz);
  auto const vertMask = (1 << vert);
  auto const diagMask = (1 << diag);

  if(nb & horzMask)
  {
    if(nb & vertMask)
    {
      if(nb & diagMask)
        return TILECLASS_PLAIN;
      else
        return TILECLASS_INNER_CORNER;
    }
    else
    {
      return TILECLASS_HORZ_WALL;
    }
  }
  else
  {
    if(nb & vertMask)
      return TILECLASS_VERT_WALL;
    else
      return TILECLASS_CORNER;
  }
}

//
// Returns the composition of one tile, given its neighboors,
// as the array of its four sub-tiles:
// .---.---.
// | 2 | 3 |
// |---|---|
// | 0 | 1 |
// .---.---.
static
array<int, 4> computeTileComposition(int nb)
{
  array<int, 4> r;

  // bottom left sub-tile
  {
    auto const cls = classify(LM, MB, nb);
    int const tiles[] =
    {
      10, 13, 12, 7, 0
    };
    r[0] = tiles[cls];
  }

  // bottom right sub-tile
  {
    auto const cls = classify(RM, MB, nb);
    int const tiles[] =
    {
      11, 5, 12, 6, 0
    };
    r[1] = tiles[cls];
  }

  // top left sub-tile
  {
    auto const cls = classify(LM, MT, nb);
    int const tiles[] =
    {
      2, 13, 4, 15, 0
    };
    r[2] = tiles[cls];
  }

  // top right sub-tile
  {
    auto const cls = classify(RM, MT, nb);
    int const tiles[] =
    {
      3, 5, 4, 14, 0
    };
    r[3] = tiles[cls];
  }

  return r;
}

array<int, 4> computeTileFor(TileMap const& occupancy, int x, int y)
{
  auto const neighboors = computeNeighboors(occupancy, x, y);

  return computeTileComposition(neighboors);
}

