/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "entity_factory.h"
#include <map>
#include <functional>

using namespace std;

static const map<string, CreationFunc> registry = getRegistry();

unique_ptr<Entity> createEntity(string formula)
{
  EntityArgs args;
  auto name = formula;

  auto i_func = registry.find(name);

  if(i_func == registry.end())
    throw runtime_error("unknown entity type: '" + name + "'");

  return (*i_func).second(args);
}

