#pragma once

#include "game.h"

struct Room
{
  Vector2i pos;
  Size2i size;
  int visualTheme = 0;
  Matrix<int> tiles;
  Vector2i start;
  std::string name;

  struct Thing
  {
    Vector2f pos;
    std::string name;
  };

  vector<Thing> things;
};

Room Graph_loadRoom(int levelIdx, IGame* game);

