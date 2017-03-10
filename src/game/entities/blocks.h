#pragma once

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"

struct CrumbleBlock : Entity
{
  CrumbleBlock()
  {
    size = Size2f(1, 1);
    collisionGroup = (1 << 1);
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
    if(other->pos.y > pos.y + size.height)
      disappearTimer = 1000;
  }

  void tick() override
  {
    decrement(disappearTimer);

    if(disappearTimer > 0)
    {
      collidesWith = 0;

      if(disappearTimer < 900)
        solid = 0;
    }
    else
    {
      collidesWith = 1; // only the player
      solid = 1;
    }
  }

  int disappearTimer = 0;
};

struct FragileBlock : Entity
{
  FragileBlock()
  {
    size = Size2f(1, 1);
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

  virtual void onDamage(int) override
  {
    disappearTimer = 3000;
  }

  void tick() override
  {
    decrement(disappearTimer);

    if(disappearTimer > 0)
    {
      collisionGroup = 0;
      solid = 0;
    }
    else
    {
      collisionGroup = -1;
      solid = 1;
    }
  }

  int disappearTimer = 0;
};

