// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"
#include "gameplay/player.h"
#include "gameplay/toggle.h"
#include "gameplay/vec.h"

namespace
{
struct FragileBlock : Entity, Damageable
{
  FragileBlock(IEntityConfig* cfg)
  {
    model = MDL_TILES_00 + cfg->getInt("theme", 0);
    tile = cfg->getInt("tile", 1);
    size = UnitSize;
    reappear();
  }

  void addActors(IActorSink* sink) const override
  {
    if(state == 2)
      return;

    auto r = SpriteActor { pos + size / 2, model };
    r.scale = size;

    if(state == 0)
      r.ratio = 0;
    else
      r.ratio = 1.0f - (timer / 50.0f);

    r.action = tile;

    sink->sendActor(r);
  }

  void onDamage(int) override
  {
    if(state != 0)
      return;

    disappear();
  }

  void tick() override
  {
    bool canReapear = !physics->getBodiesInBox(getBox(), CG_PLAYER, false, this);

    if(state == 1)
    {
      if(decrement(timer))
      {
        collisionGroup = 0;
        collidesWith = 0;
        solid = 0;

        state = 2;
        timer = 200;
      }
    }
    else if(state == 2)
    {
      decrement(timer);

      if(canReapear && timer == 0)
      {
        // reveal block
        model = MDL_BLOCK;
        tile = 5;

        reappear();
      }
    }
    else if(state == 0)
    {
      if(!canReapear)
        disappear();
    }
  }

  void reappear()
  {
    collisionGroup = CG_WALLS;
    collidesWith = CG_WALLS;
    solid = 1;
    state = 0;
  }

  void disappear()
  {
    timer = 1;
    state = 1;
  }

  int state = 0; // 0: solid, 1:disapearing, 2: disapeared
  int timer = 0;

  int model;
  int tile;
};

struct FakeBlock : Entity
{
  FakeBlock(IEntityConfig* cfg)
  {
    model = MDL_TILES_00 + cfg->getInt("theme", 0);
    tile = cfg->getInt("tile", 1);
    size = UnitSize;
    solid = false;
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, model };
    r.scale = size;
    r.ratio = 0;
    r.action = tile;

    sink->sendActor(r);
  }

  int model;
  int tile;
};

struct CrumbleBlock : Entity
{
  CrumbleBlock(IEntityConfig* cfg)
  {
    model = MDL_TILES_00 + cfg->getInt("theme", 0);
    tile = cfg->getInt("tile", 0);

    size = UnitSize;
    collisionGroup = CG_WALLS;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(IActorSink* sink) const override
  {
    if(solid)
    {
      auto r = SpriteActor { pos + size / 2, model };
      r.scale = size;
      r.action = tile;

      sink->sendActor(r);
    }
  }

  void onCollide(Body* other)
  {
    if(openingTimer)
      return;

    if(other->pos.y > pos.y + size.y * 0.9)
    {
      openingTimer = OPEN_DURATION;

      // reveal block
      model = MDL_BLOCK;
      tile = 4;
    }
  }

  void tick() override
  {
    decrement(openingTimer);

    if(openingTimer > 0)
    {
      collidesWith = 0;

      if(openingTimer < (OPEN_DURATION * 95) / 100)
        solid = 0;
    }
    else if(!physics->getBodiesInBox(getBox(), CG_PLAYER, false, this))
    {
      collidesWith = CG_PLAYER;
      solid = 1;
    }
  }

  int openingTimer = 0;
  enum { OPEN_DURATION = 40 };

  int model;
  int tile;
};

DECLARE_ENTITY("fragile_block", FragileBlock);
DECLARE_ENTITY("fake_block", FakeBlock);
DECLARE_ENTITY("crumble_block", CrumbleBlock);
}

