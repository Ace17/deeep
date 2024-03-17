// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/delegate.h"
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

using LoadTextureFunc = Delegate<int (String path, Rect2f frect)>;

Model loadModel(String path, const LoadTextureFunc& loadTexture);

