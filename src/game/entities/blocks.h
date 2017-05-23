#pragma once

#include "base/util.h"
#include "base/scene.h"
#include "game/collision_groups.h"
#include "game/entity.h"
#include "game/models.h"

struct CrumbleBlock : Entity
{
  CrumbleBlock()
  {
    size = Size(1, 1);
    collisionGroup = CG_WALLS;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = size;
    r.ratio = 0;
    r.action = 3;

    if(!solid)
      r.scale = Size(0.01, 0.01);

    return r;
  }

  void onCollide(Entity* other) override
  {
    if(other->pos.y > pos.y + size.height * 0.9)
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
    else if(!physics->getBodiesInBox(getBox(), CG_PLAYER, false, this))
    {
      collidesWith = CG_PLAYER;
      solid = 1;
    }
  }

  int disappearTimer = 0;
};

struct FragileBlock : Entity, Damageable
{
  FragileBlock()
  {
    size = Size(1, 1);
    reappear();
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = size;
    r.ratio = 0;
    r.action = solid ? 4 : 1;

    return r;
  }

  virtual void onDamage(int) override
  {
    disappear();
  }

  void tick() override
  {
    if(decrement(disappearTimer))
    {
      reappear();

      if(physics->getBodiesInBox(getBox(), CG_PLAYER, false, this))
        disappear();
    }
  }

  void reappear()
  {
    collisionGroup = CG_WALLS;
    solid = 1;
  }

  void disappear()
  {
    disappearTimer = 3000;
    collisionGroup = 0;
    solid = 0;
  }

  int disappearTimer = 0;
};

