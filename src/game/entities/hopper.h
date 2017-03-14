#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/entities/explosion.h"

struct Hopper : Entity
{
  Hopper()
  {
    dir = -1.0f;
    size = Size2f(1, 0.5);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
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

    if(dir > 0)
      r.scale.width = -r.scale.width;

    return r;
  }

  virtual void tick() override
  {
    ++time;

    vel.y -= 0.00005; // gravity

    if(time % 500 == 0 && rand() % 4 == 0)
    {
      vel.y = 0.013;
      ground = false;
    }

    if(ground)
      vel.x = 0;
    else
      vel.x = dir * 0.003;

    // horizontal move
    if(!move(this, Vector2f(vel.x, 0)))
      dir = -dir;

    // vertical move
    if(!move(this, Vector2f(0, vel.y)))
    {
      ground = true;
      vel.y = 0;
    }

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
  bool ground = false;
  float dir;
};

