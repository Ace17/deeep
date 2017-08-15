/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

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

