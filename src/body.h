// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// A collidable body inside the physics engine.
// Bodies are always axis-aligned boxes.

#pragma once

#include "vec.h"
#include <functional>

using namespace std;

IntBox roundBox(Box b);

struct Body
{
  // make type polymorphic
  virtual ~Body() = default;

  bool solid = false;
  bool pusher = false; // push and crush?
  Vector pos;

  // shape used for collision detection
  Size size = UnitSize;

  // collision masks
  int collisionGroup = 1;
  int collidesWith = 0xFFFF;

  // the body we rest on (if any)
  Body* floor = nullptr;

  // only called if (this->collidesWith & other->collisionGroup)
  function<void(Body*)> onCollision = [] (Body*) {};

  Box getFBox() const { return Box { pos, size }; }
  IntBox getBox() const { return roundBox(getFBox()); }
};

