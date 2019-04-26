// Copyright (C) 2019 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <functional>
#include <string>
#include <memory>

using namespace std;

struct Entity;

// e.g:
// createEntity("spider");
// createEntity("door(4)");
std::unique_ptr<Entity> createEntity(string name);

struct EntityConfig
{
  virtual string getString(const char* varName) = 0;

  int getInt(const char* varName) { return atoi(getString(varName).c_str()); }
};

using CreationFunc = function<unique_ptr<Entity>(EntityConfig & args)>;
int registerEntity(string type, CreationFunc func);

