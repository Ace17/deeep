#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/entities/player.h"

class Wheel : public Entity
{
public:
  Wheel()
  {
    dir = -1.0f;
    size = Size2f(1.0, 1.0);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_WHEEL);

    r.scale = Vector2f(3, 3);
    r.pos += Vector2f(-(r.scale.x - size.width) * 0.5, -0.1);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    r.action = 0;
    r.ratio = (time % 200) / 200.0f;

    if(dir > 0)
      r.scale.x = -r.scale.x;

    return r;
  }

  bool move(Vector2f delta)
  {
    auto nextPos = pos + delta;

    if(game->isSolid(nextPos + Vector2f(0, 0)))
      return false;

    if(game->isSolid(nextPos + Vector2f(size.width, 0)))
      return false;

    if(game->isSolid(nextPos + Vector2f(0, size.height)))
      return false;

    if(game->isSolid(nextPos + Vector2f(size.width, size.height)))
      return false;

    pos = nextPos;
    return true;
  }

  virtual void tick() override
  {
    ++time;

    vel.x = dir * 0.003;
    vel.y -= 0.00005; // gravity

    // horizontal move
    if(!move(Vector2f(vel.x, 0)))
      dir = -dir;

    // vertical move
    if(!move(Vector2f(0, vel.y)))
      vel.y = 0;

    blinking = max(0, blinking - 1);
  }

  virtual void onCollide(Entity* other) override
  {
    if(dynamic_cast<Player*>(other))
      other->onDamage(3);
  }

  virtual void onDamage(int) override
  {
    blinking = 100;
  }

  Int time;
  float dir;
};

