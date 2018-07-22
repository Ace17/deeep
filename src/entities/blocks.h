/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "vec.h"
#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "models.h"

struct CrumbleBlock : Entity
{
  CrumbleBlock()
  {
    size = UnitSize;
    collisionGroup = CG_WALLS;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    if(!solid)
      return;

    auto r = Actor(pos, MDL_BLOCK);
    r.scale = size;
    r.ratio = 0;
    r.action = 1;

    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    if(other->pos.y > pos.y + size.height * 0.9)
      disappearTimer = 1000;
  }

  void tick() override
  {
    decrement(disappearTimer);

    if(disappearTimer > 0)
    {
      collidesWith = 0;

      if(disappearTimer < 900)
        solid = 0;
    }
    else if(!physics->getBodiesInBox(getBox(), CG_PLAYER, false, this))
    {
      collidesWith = CG_PLAYER;
      solid = 1;
    }
  }

  int disappearTimer = 0;
};

struct FragileBlock : Entity, Damageable
{
  FragileBlock()
  {
    size = UnitSize;
    reappear();
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor(pos, MDL_BLOCK);
    r.scale = size;
    r.ratio = 0;
    r.action = solid ? 2 : 0;

    actors.push_back(r);
  }

  virtual void onDamage(int) override
  {
    disappear();
  }

  void tick() override
  {
    if(decrement(disappearTimer))
    {
      reappear();

      if(physics->getBodiesInBox(getBox(), CG_PLAYER, false, this))
        disappear();
    }
  }

  void reappear()
  {
    collisionGroup = CG_WALLS;
    solid = 1;
  }

  void disappear()
  {
    disappearTimer = 3000;
    collisionGroup = 0;
    solid = 0;
  }

  int disappearTimer = 0;
};

