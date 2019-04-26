// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <string>
#include <map>
#include <vector>
#include "base/geom.h"
#include "vec.h"

using namespace std;

// A room (i.e a level)
struct Room
{
  Vector2i pos;
  Size2i size;
  int theme = 0;
  Matrix2<int> tiles;
  Matrix2<int> tilesForDisplay;
  Vector2i start;
  std::string name;

  struct Spawner
  {
    Vector pos;
    std::string name;
    std::map<std::string, std::string> config {};
  };

  vector<Spawner> spawners;
};

// The whole game
struct Quest
{
  vector<Room> rooms;
};

