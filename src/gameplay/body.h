// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// A collidable body inside the physics engine.
// Bodies are always axis-aligned boxes.

#pragma once

#include "base/delegate.h"
#include "vec.h"

using namespace std;

struct Body;

struct Shape
{
  virtual ~Shape() = default;
  virtual bool probe(Body* shapeOwner, Box otherBox) const = 0;
  virtual float raycast(Body* shapeOwner, Box otherBox, Vector2f delta) const = 0;
};

struct Body
{
  Body();
  // make type polymorphic
  virtual ~Body() = default;

  bool solid = false;
  bool pusher = false; // push and crush?
  Vector pos;

  // shape used for collision detection
  Size size = UnitSize;
  const Shape* shape;

  // collision masks
  int collisionGroup = 1;
  int collidesWith = 0xFFFF;

  // the body we rest on (if any)
  Body* floor = nullptr;

  // only called if (this->collidesWith & other->collisionGroup)
  Delegate<void(Body*)> onCollision = [] (Body*) {};

  Box getBox() const { return Box { pos, size }; }
};

struct ShapeBox : Shape
{
  bool probe(Body* shapeOwner, Box otherBox) const override;
  float raycast(Body* shapeOwner, Box otherBox, Vector2f delta) const override;
};

struct ShapeTilemap : Shape
{
  bool probe(Body* shapeOwner, Box otherBox) const override;
  float raycast(Body* shapeOwner, Box otherBox, Vector2f delta) const override;
  Matrix2<int>* tiles;
};

