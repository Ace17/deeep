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
  Entity()
  {
    size = Size2f(1, 1);
    game = nullptr;
  }

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

  Bool dead;
  Bool solid;
  Vector2f pos;
  Vector2f vel;
  Size2f size;
  Int blinking;
  IGame* game;

  Int collisionGroup = 1;
  Int collidesWith = 0xFFFF;

  Rect2f getRect() const
  {
    Rect2f r;
    r.x = pos.x;
    r.y = pos.y;
    r.height = size.height;
    r.width = size.width;
    return r;
  }
};

