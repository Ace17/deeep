// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <cmath> // sin

#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "toggle.h"
#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "entities/move.h"

struct MovingPlatform : Entity
{
  MovingPlatform(int dir_, int speed_)
  {
    speed = speed_ / 100.0;
    solid = true;
    pusher = true;
    size = Size(2, 1);
    collisionGroup = CG_WALLS;
    ticks = rand();
    dir = dir_;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_RECT };
    r.scale = size;

    actors.push_back(r);
  }

  void tick() override
  {
    auto delta = 0.05 * sin(ticks * 0.05) * speed;

    // moveBody doesn't work well with very small displacements
    if(abs(delta) < 0.001)
      delta = 0;

    auto v = dir ? Vector(delta, 0) : Vector(0, delta);
    physics->moveBody(this, v);
    ++ticks;
  }

  int ticks = 0;
  int dir = 0;
  float speed = 1.0;
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

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_ELEVATOR };
    r.scale = size;
    r.ratio = 0;
    r.action = liftTimer > 0 ? 1 : 0;

    if(liftTimer > 0)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    if(other->pos.y > pos.y + size.height / 2)
    {
      if(debounceTrigger == 0)
        trigger();

      debounceTrigger = 100;
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
    for(int i = 0; i < 10; ++i)
      subTick();
  }

  void subTick()
  {
    decrement(liftTimer);
    decrement(debounceTrigger);

    auto liftVel = Vector(0, 0.00466);

    if(liftTimer >= 4500)
    {
      // initial pause
    }
    else if(liftTimer >= 3000)
    {
      // go up
      physics->moveBody(this, liftVel);
    }
    else if(liftTimer >= 2000)
    {
      // pause in top position
      auto topPos = initialPos + Vector(0, 7);
      physics->moveBody(this, topPos - pos);
    }
    else if(liftTimer > 500)
    {
      // go down
      physics->moveBody(this, -1.0 * liftVel);
    }
    else if(liftTimer < 500)
    {
      // stuck in initial position
      physics->moveBody(this, initialPos - pos);
    }
  }

  int liftTimer = 0;
  int debounceTrigger = 0;

  Vector initialPos;
};

#include "entity_factory.h"

static auto const reg1 = registerEntity("moving_platform", [] (IEntityConfig* cfg) { auto arg = cfg->getInt("dir"); auto speed = cfg->getInt("speed", 100); return make_unique<MovingPlatform>(arg, speed); });
static auto const reg2 = registerEntity("elevator", [] (IEntityConfig*) { return make_unique<Elevator>(); });

