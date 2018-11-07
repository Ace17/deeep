// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "vec.h"
#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "models.h"

struct FragileBlock : Entity, Damageable
{
  FragileBlock()
  {
    size = UnitSize;
    reappear();
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    if(!solid)
      return;

    auto r = Actor { pos, MDL_BLOCK };
    r.scale = size;
    r.ratio = 0;
    r.action = 3;

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

