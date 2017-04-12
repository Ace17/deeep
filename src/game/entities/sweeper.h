#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/entities/explosion.h"

struct Sweeper : Entity, Damageable
{
  Sweeper()
  {
    size = Size2f(0.8, 0.8);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;

    vel.x = 0.003;
    vel.y = 0.003;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);

    r.scale = size;
    r.pos += Vector2f(-(r.scale.width - size.width) * 0.5, 0);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    r.action = 0;
    r.ratio = (time % 800) / 800.0f;

    return r;
  }

  virtual void tick() override
  {
    ++time;

    auto trace = slideMove(this, vel);

    if(!trace.horz)
      vel.x = -vel.x;

    if(!trace.vert)
      vel.y = -vel.y;

    decrement(blinking);
  }

  virtual void onCollide(Entity* other) override
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
};

