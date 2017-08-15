#pragma once

#include "game/entity.h"
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

