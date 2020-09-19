// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h"
#include "gameplay/player.h"
#include "gameplay/toggle.h"
#include "gameplay/vec.h"

struct FragileBlock : Entity, Damageable
{
  FragileBlock()
  {
    size = UnitSize;
    reappear();
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    if(state == 2)
      return;

    auto r = Actor { pos, MDL_BLOCK };
    r.scale = size;

    if(state == 0)
      r.ratio = 0;
    else
      r.ratio = 1.0f - (timer / 50.0f);

    r.action = 3;

    actors.push_back(r);
  }

  virtual void onDamage(int) override
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
        timer = 300;
      }
    }
    else if(state == 2)
    {
      decrement(timer);

      if(canReapear && timer == 0)
        reappear();
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
    timer = 50;
    state = 1;
  }

  int state = 0; // 0: solid, 1:disapearing, 2: disapeared
  int timer = 0;
};

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("fragile_block", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<FragileBlock>(); });

