#pragma once

#include "base/scene.h"
#include "game/collision_groups.h"
#include "game/entity.h"
#include "game/models.h"

struct Conveyor : Entity
{
  Conveyor()
  {
    size = Size2f(1, 1);
    collisionGroup = CG_WALLS;
    collidesWith = CG_PLAYER;
    solid = 1;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = size;
    r.ratio = 0;
    r.action = 0;

    if(!solid)
      r.scale = Size2f(0.01, 0.01);

    return r;
  }

  void onCollide(Entity* other) override
  {
    physics->moveBody(other, Vector2f(-0.004, 0));
  }
};

