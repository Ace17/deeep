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

#include "engine/scene.h"
#include "engine/geom.h"
#include "game.h"

class Entity
{
public:
  Entity()
  {
    size = Dimension2f(1, 1);
    game = nullptr;
  }

  virtual ~Entity()
  {
  }

  virtual Actor getActor() const = 0;
  virtual void tick()
  {
  }

  virtual void onCollide(Entity*)
  {
  }

  virtual void onDamage(int /*amount*/)
  {
  }

  Bool dead;
  Bool solid;
  Vector2f pos;
  Vector2f vel;
  Dimension2f size;
  Int blinking;
  IGame* game;

  Rect2f getRect() const
  {
    Rect2f r;
    r.x = pos.x - size.width / 2;
    r.y = pos.y - size.height / 2;
    r.height = size.height;
    r.width = size.width;
    return r;
  }
};

