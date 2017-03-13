/**
 * @brief Entity: base game object
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "base/scene.h"
#include "base/geom.h"
#include "game.h"

struct Entity
{
  virtual ~Entity()
  {
  }

  virtual Actor getActor() const = 0;
  virtual void tick()
  {
  }

  // only called if (this->collidesWith & other->collisionGroup)
  virtual void onCollide(Entity* /*other*/)
  {
  }

  virtual void onDamage(int /*amount*/)
  {
  }

  bool dead = false;
  bool solid = false;
  Vector2f pos;
  Vector2f vel;
  Size2f size = Size2f(1, 1);
  int blinking = 0;
  IGame* game = nullptr;

  int collisionGroup = 1;
  int collidesWith = 0xFFFF;

  Rect2f getRect() const
  {
    Rect2f r;
    r.x = pos.x;
    r.y = pos.y;
    r.height = size.height;
    r.width = size.width;
    return r;
  }

  Vector2f getCenter() const
  {
    return Vector2f(pos.x + size.width / 2, pos.y + size.height / 2);
  }
};

