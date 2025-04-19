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
#include "gameplay/player.h"
#include "gameplay/toggle.h"
#include "gameplay/vec.h"

namespace
{
struct ExitPoint : Entity
{
  ExitPoint(IEntityConfig*)
  {
    solid = 0;
    size = UnitSize;
    collisionGroup = 0;
    collidesWith = CG_PLAYER | CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_BLOCK };
    r.scale = size;
    sink->sendActor(r);
  }

  void onCollide(Body* other)
  {
    if(!active)
      return;

    if(dynamic_cast<Playerable*>(other))
    {
      game->stopMusic();
      active = false;
      timer = 50;
    }
  }

  void tick() override
  {
    if(active)
      return;

    game->setAmbientLight(1.0 - timer / 50.0);

    if(decrement(timer))
    {
      game->postEvent(std::make_unique<FinishGameEvent>());
      active = true;
    }
  }

  bool active = true;
  int timer = 0;
};

DECLARE_ENTITY("exitpoint", ExitPoint);
}

