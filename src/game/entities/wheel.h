#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/collision_groups.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/toggle.h"
#include "game/entities/explosion.h"
#include "game/entities/move.h"

struct Wheel : Entity
{
  Wheel()
  {
    dir = -1.0f;
    size = Size2f(1.5, 1.5);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_WHEEL);

    r.scale = Size2f(3, 3);
    r.pos += Vector2f(-(r.scale.width - size.width) * 0.5, -0.3);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    r.action = 0;
    r.ratio = (time % 200) / 200.0f;

    if(dir > 0)
      r.scale.width = -r.scale.width;

    return r;
  }

  virtual void tick() override
  {
    ++time;

    vel.x = dir * 0.003;
    vel.y -= 0.00005; // gravity

    auto trace = slideMove(this, vel);

    if(!trace.horz)
      dir = -dir;

    if(!trace.vert)
      vel.y = 0;

    decrement(blinking);
  }

  virtual void onCollide(Entity* other) override
  {
    other->onDamage(5);
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
  float dir;
};

