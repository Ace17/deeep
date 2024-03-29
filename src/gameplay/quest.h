// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/geom.h"
#include "base/matrix.h"
#include "vec.h"
#include <map>
#include <string>
#include <vector>

// A room (i.e a level)
struct Room
{
  Vec2i pos;
  Vec2i size; // in cells
  int theme = 0;
  Matrix2<int> tiles;
  Matrix2<int> tilesForDisplay;
  Vec2i start;
  std::string name;

  struct Spawner
  {
    int id;
    Vector pos;
    std::string name;
    std::map<std::string, std::string> config {};
  };

  std::vector<Spawner> spawners;
};

// The whole game
struct Quest
{
  std::vector<Room> rooms;
};

