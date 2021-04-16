// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/string.h"
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

Model loadModel(String path);

