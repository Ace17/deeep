// Copyright (C) 2023 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Pluggable entity factory, registration side.

#include "base/error.h"
#include "entity.h"
#include "entity_factory.h"
#include <map>

namespace
{
struct EntityInfo
{
  CreationFunc creationFunc;
  uint32_t flags;
};

std::map<std::string, EntityInfo>& g_registry()
{
  static std::map<std::string, EntityInfo> registry;
  return registry;
}

const EntityInfo& getEntityInfo(std::string name)
{
  auto i_info = g_registry().find(name);

  if(i_info == g_registry().end())
    throw Error("unknown entity type: '" + name + "'");

  return i_info->second;
}
}

int registerEntity(std::string type, CreationFunc func, uint32_t flags)
{
  auto& info = g_registry()[type];
  info.creationFunc = func;
  info.flags = flags;
  return 0; // ignored
}

uint32_t getEntityFlags(std::string name)
{
  return getEntityInfo(name).flags;
}

std::unique_ptr<Entity> createEntity(std::string name, IEntityConfig* args)
{
  return getEntityInfo(name).creationFunc(args);
}

