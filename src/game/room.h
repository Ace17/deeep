#pragma once

#include "game.h"

struct Room
{
  Vector2i pos;
  Size2i size;
  int theme = 0;
  Matrix2<int> tiles;
  Vector2i start;
  std::string name;

  struct Thing
  {
    Vector pos;
    std::string name;
  };

  vector<Thing> things;
};

Room Graph_loadRoom(int levelIdx, IGame* game);

