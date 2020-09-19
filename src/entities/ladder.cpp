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

struct Ladder : Entity, Climbable
{
  Ladder()
  {
    size = UnitSize;
    solid = 0;
    collisionGroup = CG_LADDER;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_LADDER };
    r.scale = size;
    r.ratio = 0;
    r.action = 6;
    actors.push_back(r);
  }
};

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("ladder", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<Ladder>(); });

