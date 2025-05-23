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
#include "gameplay/models.h" // MDL_BLOCK
#include "gameplay/sounds.h" // SND_HATCH
#include "gameplay/toggle.h" // decrement
#include "gameplay/vec.h"

namespace
{
struct Hatch : Entity
{
  Hatch(IEntityConfig*)
  {
    size = UnitSize;
    collisionGroup = CG_WALLS;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_BLOCK };
    r.scale = size;
    r.ratio = 1.0f - openingTimer / float(OPEN_DURATION);
    r.action = solid ? 1 : 2;

    sink->sendActor(r);
  }

  void onCollide(Body* other)
  {
    if(openingTimer)
      return;

    if(other->pos.y > pos.y + size.y * 0.9)
    {
      openingTimer = OPEN_DURATION;
      game->playSound(SND_HATCH);
    }
  }

  void tick() override
  {
    decrement(openingTimer);

    if(openingTimer > 0)
    {
      collidesWith = 0;

      if(openingTimer < (OPEN_DURATION * 9) / 10)
        solid = 0;
    }
    else if(!physics->getBodiesInBox(getBox(), CG_PLAYER, false, this))
    {
      collidesWith = CG_PLAYER;
      solid = 1;
    }
  }

  int openingTimer = 0;
  enum { OPEN_DURATION = 100 };
};

DECLARE_ENTITY("hatch", Hatch);
}

