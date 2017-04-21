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
    r.action = 2;
    r.scale = size;
    r.scale.width *= -1;
    return r;
  }

  void onCollide(Entity* other) override
  {
    // avoid infinite recursion
    // (if the conveyor pushes the player towards the conveyor)
    if(noRecurse)
      return;

    noRecurse = true;
    physics->moveBody(other, Vector2f(-0.004, 0));
    noRecurse = false;
  }

  bool noRecurse = false;
};

