#pragma once

#include "base/util.h"
#include "base/scene.h"
#include "game/collision_groups.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/entities/move.h"

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
  }

  void enter() override
  {
    pos.x += 0.001;
    size.width -= 0.002;
    initialPos = pos;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = size;
    r.ratio = 0;
    r.action = 4;
    if(liftTimer > 0)
      r.effect = Effect::Blinking;

    return r;
  }

  void onCollide(Entity* other) override
  {
    if(other->pos.y > pos.y + size.height)
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

