// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Pluggable entity factory, registration side.

#include "base/error.h"
#include "entity.h"
#include "entity_factory.h"
#include <map>

using namespace std;

namespace
{
map<string, CreationFunc> & g_registry()
{
  static map<string, CreationFunc> registry;
  return registry;
}
}

int registerEntity(string type, CreationFunc func)
{
  g_registry()[type] = func;
  return 0; // ignored
}

unique_ptr<Entity> createEntity(string name, IEntityConfig* args)
{
  auto i_func = g_registry().find(name);

  if(i_func == g_registry().end())
    throw Error("unknown entity type: '" + name + "'");

  return (*i_func).second(args);
}

