#pragma once

#include "game.h"

struct Level
{
  Vector2i pos;
  Size2i size;
  Matrix<int> tiles;
  Vector2i start;
};

Level Graph_loadLevel(int levelIdx, IGame* game);

