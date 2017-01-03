#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/entities/player.h"

static auto const DURATION = 400;

struct Explosion : public Entity
{
  Explosion()
  {
    size = Size2f(0.1, 0.1);
  }

  virtual void tick() override
  {
    time++;

    if(time >= DURATION)
    {
      time = DURATION;
      dead = true;
    }
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_EXPLOSION);

    r.ratio = time / (float)DURATION;
    r.scale = Vector2f(3, 3);
    r.pos += Vector2f(-r.scale.x * 0.5, -r.scale.y * 0.5);

    return r;
  }

  Int time;
};

