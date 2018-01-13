/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <cmath> // sin

#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "models.h"
#include "entities/move.h"

struct MovingPlatform : Entity
{
  MovingPlatform(int dir_)
  {
    solid = true;
    pusher = true;
    size = Size(2, 1);
    collisionGroup = CG_WALLS;
    ticks = rand();
    dir = dir_;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = size;

    return r;
  }

  void tick() override
  {
    auto delta = 0.005 * sin(ticks * 0.005);

    // moveBody doesn't work well with very small displacements
    if(abs(delta) < 0.001)
      delta = 0;

    auto v = dir ? Vector(delta, 0) : Vector(0, delta);
    physics->moveBody(this, v);
    ++ticks;
  }

  int ticks = 0;
  int dir = 0;
};

struct Elevator : Entity
{
  Elevator()
  {
    solid = true;
    pusher = true;
    size = Size(2, 1);
    collisionGroup = CG_WALLS;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void enter() override
  {
    pos.x += 0.001;
    size.width -= 0.002;
    initialPos = pos;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_ELEVATOR);
    r.scale = size;
    r.ratio = 0;
    r.action = liftTimer > 0 ? 1 : 0;

    if(liftTimer > 0)
      r.effect = Effect::Blinking;

    return r;
  }

  void onCollide(Body* other)
  {
    if(other->pos.y > pos.y + size.height / 2)
    {
      if(triggeredTime == 0)
        trigger();

      triggeredTime = 100;
    }
  }

  void trigger()
  {
    if(liftTimer > 0)
      return;

    game->playSound(SND_SWITCH);
    liftTimer = 5000;
  }

  void tick() override
  {
    decrement(liftTimer);
    decrement(triggeredTime);

    if(liftTimer > 500 && liftTimer < 4500)
    {
      auto sign = liftTimer > 2500 ? 1 : -1;
      physics->moveBody(this, Vector(0, sign * 0.004));
    }
    else
    {
      physics->moveBody(this, initialPos - pos);
    }
  }

  int liftTimer = 0;
  int triggeredTime = 0;

  Vector initialPos;
};

