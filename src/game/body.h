/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <functional>
#include "base/geom.h"

typedef GenericVector<float> Vector;
typedef GenericSize<float> Size;
typedef GenericBox<float> Box;
typedef GenericBox<int> IntBox;

static auto const UnitSize = Size(1, 1);

using namespace std;

static auto const PRECISION = 1024;
IntBox roundBox(Box b);

struct Body
{
  bool solid = false;
  bool pusher = false; // push and crush?
  Vector pos;
  Size size = Size(1, 1);
  int collisionGroup = 1;
  int collidesWith = 0xFFFF;
  Body* ground = nullptr; // the body we rest on (if any)

  // only called if (this->collidesWith & other->collisionGroup)
  virtual void onCollision(Body* /*other*/)
  {
  }

  Box getFBox() const
  {
    Box r;
    r.pos = pos;
    r.size = size;
    return r;
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

