// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h" // MDL_BLOCK
#include "gameplay/player.h"
#include "gameplay/sounds.h" // SND_HATCH
#include "gameplay/toggle.h"
#include "gameplay/vec.h"

struct SavePoint : Entity
{
  SavePoint()
  {
    solid = 0;
    size = UnitSize;
    collisionGroup = 0;
    collidesWith = CG_PLAYER | CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_SAVEPOINT };
    r.scale = size;
    r.ratio = 0;
    r.action = 0;

    if(timer)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  void tick() override
  {
    decrement(timer);
  }

  void onCollide(Body* other)
  {
    if(dynamic_cast<Player*>(other) && !timer)
    {
      game->playSound(SND_SAVEPOINT);
      game->postEvent(make_unique<SaveEvent>());
      game->textBox("Game Saved");
      timer = 300;
    }
  }

  int timer = 0;
};

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("savepoint", [] (IEntityConfig*)  -> unique_ptr<Entity> { return make_unique<SavePoint>(); });

