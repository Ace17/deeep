/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

struct Entity;

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <map>

using namespace std;

// e.g:
// createEntity("spider");
// createEntity("door(4)");
std::unique_ptr<Entity> createEntity(string name);

using EntityConfig = vector<string> const;
using CreationFunc = function<unique_ptr<Entity>(EntityConfig & args)>;

// user-provided
extern map<string, CreationFunc> getRegistry();

