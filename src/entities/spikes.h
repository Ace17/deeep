/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "base/util.h"
#include "base/scene.h"

#include "entity.h"
#include "models.h"

struct Spikes : Entity
{
  Spikes()
  {
    size = Size(1, 0.95);
    solid = 1;
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SPIKES);
    r.scale = size;
    r.ratio = 0;

    return r;
  }

  void onCollide(Entity* other) override
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(1000);
  }
};
