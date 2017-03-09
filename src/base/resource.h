#pragma once

#include "util.h"

struct Resource
{
  int id;
  char const* path;
};

Span<const Resource> getSounds();
Span<const Resource> getModels();

