/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "entity.h"
#include <functional>
#include <memory>
#include <map>

// e.g:
// createEntity("spider");
// createEntity("door(4)");
std::unique_ptr<Entity> createEntity(string name);

typedef vector<string> const EntityArgs;
typedef function<unique_ptr<Entity>(EntityArgs & args)> CreationFunc;

// user-provided
extern map<string, CreationFunc> getRegistry();

