/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"

#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "entities/explosion.h"

struct Sweeper : Entity, Damageable
{
  Sweeper()
  {
    size = Size(0.8, 0.8);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };

    vel.x = 0.003;
    vel.y = 0.003;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SWEEPER);

    r.scale = size;
    r.pos += Vector(-(r.scale.width - size.width) * 0.5, 0);

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = 0;
    r.ratio = (time % 800) / 800.0f;

    return r;
  }

  virtual void tick() override
  {
    ++time;

    auto trace = slideMove(this, vel);

    if(!trace.horz)
      vel.x = -vel.x;

    if(!trace.vert)
      vel.y = -vel.y;

    decrement(blinking);
  }

  void onCollide(Body* other)
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(5);
  }

  virtual void onDamage(int amount) override
  {
    blinking = 750;
    life -= amount;

    game->playSound(SND_DAMAGE);

    if(life < 0)
    {
      game->playSound(SND_EXPLODE);
      dead = true;

      auto explosion = makeExplosion();
      explosion->pos = getCenter();
      game->spawn(explosion.release());
    }
  }

  int life = 30;
  int time = 0;
};

