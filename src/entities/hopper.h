/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
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
#include "sounds.h"
#include "entities/explosion.h"

#include <cstdlib> // rand

struct Hopper : Entity, Damageable
{
  Hopper()
  {
    vel = Vector2f(0, 0);
    dir = -1.0f;
    size = Size(1, 0.5);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);

    r.scale = size;
    r.pos += Vector(-(r.scale.width - size.width) * 0.5, 0);

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = 0;
    r.ratio = (time % 800) / 800.0f;

    if(dir > 0)
      r.scale.width = -r.scale.width;

    return r;
  }

  virtual void tick() override
  {
    ++time;

    vel.y -= 0.00005; // gravity

    if(ground && time % 500 == 0 && rand() % 4 == 0)
    {
      vel.y = 0.013;
      ground = false;
    }

    if(ground)
      vel.x = 0;
    else
      vel.x = dir * 0.003;

    auto trace = slideMove(this, vel);

    if(!trace.horz)
      dir = -dir;

    if(!trace.vert)
    {
      ground = true;
      vel.y = 0;
    }

    decrement(blinking);
  }

  void onCollide(Body* other)
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(5);
  }

  virtual void onDamage(int amount) override
  {
    blinking = 100;
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
  bool ground = false;
  float dir;
  Vector vel;
};

