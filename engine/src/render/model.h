// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <stdint.h>
#include <vector>
#include <string>
using namespace std;

#include "base/geom.h"

struct Action
{
  vector<int> textures;
};

struct Model
{
  // mesh data
  struct Vertex
  {
    float x, y, u, v;
  };

  vector<Vertex> vertices;
  vector<Action> actions;
};

Model loadModel(string jsonPath);
void addTexture(Action& action, string path, Rect2i rect = Rect2i(0, 0, 0, 0));

