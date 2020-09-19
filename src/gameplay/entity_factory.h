// Copyright (C) 2019 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <memory>
#include <string>

using namespace std;

struct Entity;

struct IEntityConfig
{
  virtual string getString(const char* varName, string defaultValue = "") = 0;
  virtual int getInt(const char* varName, int defaultValue = 0) = 0;
};

// e.g:
// createEntity("spider");
// createEntity("door(4)");
std::unique_ptr<Entity> createEntity(string name, IEntityConfig* config);

using CreationFunc = unique_ptr<Entity>(*)(IEntityConfig* args);
int registerEntity(string type, CreationFunc func);

#define DECLARE_ENTITY(Name, Class) \
  DECLARE_ENTITY_COUNTER(Name, Class, __LINE__)

#define DECLARE_ENTITY_COUNTER(Name, Class, Counter) \
  static auto const reg_ ## Counter = registerEntity(Name, [] (IEntityConfig* cfg)  -> unique_ptr<Entity> { return make_unique<Class>(cfg); })

