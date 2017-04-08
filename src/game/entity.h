/**
 * @brief Entity: base game object
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "base/scene.h"
#include "base/geom.h"
#include "game.h"
#include "body.h"

struct Entity : Body
{
  virtual ~Entity()
  {
  }

  // from Body
  virtual void onCollision(Body* otherBody) override
  {
    auto other = dynamic_cast<Entity*>(otherBody);
    assert(other);
    onCollide(other);
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
  Vector2f vel;
  int blinking = 0;
  IGame* game = nullptr;
  IPhysics* physics = nullptr;

  Vector2f getCenter() const
  {
    return Vector2f(pos.x + size.width / 2, pos.y + size.height / 2);
  }
};

