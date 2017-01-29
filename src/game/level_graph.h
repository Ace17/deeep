#pragma once

#include "game.h"

struct Level
{
  Matrix<int> tiles;
  Vector2i start;
};

Level Graph_loadLevel(int levelIdx, IGame* game);

