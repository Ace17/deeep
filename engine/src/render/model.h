// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <vector>
using namespace std;

struct Action
{
  vector<int> textures;
};

struct Model
{
  vector<Action> actions;
};

Model loadModel(const char* jsonPath);

#include "base/geom.h"
void addTexture(Action& action, const char* path, Rect2i rect = Rect2i(0, 0, 0, 0));

