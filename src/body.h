/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// A collidable body inside the physics engine.
// Bodies are always axis-aligned boxes.

#pragma once

#include <functional>
#include "vec.h"

using namespace std;

IntBox roundBox(Box b);

struct Body
{
  // make type polymorphic
  virtual ~Body() = default;

  bool solid = false;
  bool pusher = false; // push and crush?
  Vector pos;
  Size size = UnitSize;
  int collisionGroup = 1;
  int collidesWith = 0xFFFF;
  Body* ground = nullptr; // the body we rest on (if any)

  // only called if (this->collidesWith & other->collisionGroup)
  function<void(Body*)> onCollision = &nop;

  static void nop(Body*) {}

  Box getFBox() const
  {
    return Box { pos, size };
  }

  IntBox getBox() const
  {
    return roundBox(getFBox());
  }
};

struct IPhysicsProbe
{
  virtual bool moveBody(Body* body, Vector delta) = 0;
  virtual bool isSolid(const Body* body, IntBox) const = 0;
  virtual Body* getBodiesInBox(IntBox myBox, int collisionGroup, bool onlySolid = false, const Body* except = nullptr) const = 0;
};

