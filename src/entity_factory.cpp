// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Pluggable entity factory, registration side.

#include "entity.h"
#include "entity_factory.h"
#include <map>
#include <stdexcept>

using namespace std;

namespace
{
// don't put a unique_ptr here, as this would make us
// depend on the module initialization order
map<string, CreationFunc>* g_registry;

struct RegistryDeleter
{
  ~RegistryDeleter() { delete g_registry; }
};

RegistryDeleter g_deleteRegistryAtProgramExit;
}

int registerEntity(string type, CreationFunc func)
{
  if(!g_registry)
    g_registry = new map<string, CreationFunc>;

  (*g_registry)[type] = func;
  return 0; // ignored
}

unique_ptr<Entity> createEntity(string name, IEntityConfig* args)
{
  auto i_func = g_registry->find(name);

  if(i_func == g_registry->end())
    throw runtime_error("unknown entity type: '" + name + "'");

  return (*i_func).second(args);
}

