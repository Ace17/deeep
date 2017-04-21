#pragma once

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"

struct Spikes : Entity
{
  Spikes()
  {
    size = Size2f(1, 0.95);
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

