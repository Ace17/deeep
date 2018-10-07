// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "vec.h"
#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "models.h" // MDL_BLOCK
#include "sounds.h" // SND_HATCH
#include "entities/player.h"

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
    auto r = Actor { pos, MDL_BLOCK };
    r.scale = size;
    r.ratio = 0;
    r.action = 0;

    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    if(dynamic_cast<Player*>(other))
    {
      game->textBox("Game Saved");
      game->savepoint();
    }
  }
};

