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
    size = Size2f(2, 1);
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
    auto v = dir ? Vector2f(delta, 0) : Vector2f(0, delta);
    physics->moveBody(this, v);
    ++ticks;
  }

  int ticks = 0;
  int dir = 0;
};

