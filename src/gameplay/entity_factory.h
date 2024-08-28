// Copyright (C) 2023 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <memory>
#include <string>

struct Entity;

struct IEntityConfig
{
  virtual std::string getString(const char* varName, std::string defaultValue = "") = 0;
  virtual int getInt(const char* varName, int defaultValue = 0) = 0;
};

enum EntityFlags : uint32_t
{
  EntityFlag_ShowOnMinimap_S = 1,
  EntityFlag_ShowOnMinimap_O = 2,
  EntityFlag_Persist = 4,
};

std::unique_ptr<Entity> createEntity(std::string name, IEntityConfig* config);
uint32_t getEntityFlags(std::string name);

using CreationFunc = std::unique_ptr<Entity>(*)(IEntityConfig* args);
int registerEntity(std::string type, CreationFunc func, uint32_t flags);

#define DECLARE_ENTITY(Name, Class) \
        DECLARE_ENTITY_COUNTER_PRE(Name, Class, __LINE__)

#define DECLARE_ENTITY_COUNTER_PRE(Name, Class, Counter) \
        DECLARE_ENTITY_COUNTER(Name, Class, Counter)

#define DECLARE_ENTITY_COUNTER(Name, Class, Counter) \
        static auto const reg_ ## Counter = registerEntity(Name, [] (IEntityConfig* cfg) -> std::unique_ptr<Entity> { return make_unique<Class>(cfg); }, Class::flags)

