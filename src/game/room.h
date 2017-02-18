#pragma once

#include "game.h"

struct Room
{
  Vector2i pos;
  Size2i size;
  Matrix<int> tiles;
  Vector2i start;
  std::string name;
};

Room Graph_loadRoom(int levelIdx, IGame* game);

