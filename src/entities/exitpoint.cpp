// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "vec.h"
#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "toggle.h"
#include "models.h" // MDL_BLOCK
#include "entities/player.h"

struct ExitPoint : Entity
{
  ExitPoint()
  {
    solid = 0;
    size = UnitSize;
    collisionGroup = 0;
    collidesWith = CG_PLAYER | CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_BLOCK };
    r.scale = size;
    r.ratio = 0;
    r.action = 0;

    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    if(!active)
      return;

    if(dynamic_cast<Player*>(other))
    {
      game->stopMusic();
      active = false;
      timer = 50;
    }
  }

  virtual void tick() override
  {
    if(active)
      return;

    game->setAmbientLight(1.0 - timer / 50.0);

    if(decrement(timer))
    {
      game->postEvent(make_unique<FinishGameEvent>());
      active = true;
    }
  }

  bool active = true;
  int timer = 0;
};

#include "entity_factory.h"
static auto const reg1 = registerEntity("exitpoint", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<ExitPoint>(); });

