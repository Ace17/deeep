// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "span.h"

enum class ResourceType
{
  Sound,
  Model,
};

struct Resource
{
  ResourceType type;
  int id;
  char const* path;
};

// must be implemented by the game
Span<const Resource> getResources();

