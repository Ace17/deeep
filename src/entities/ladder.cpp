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

namespace
{
struct Ladder : Entity, Climbable
{
  Ladder(IEntityConfig*)
  {
    size = UnitSize;
    solid = 0;
    collisionGroup = CG_LADDER;
  }

  void addActors(std::vector<SpriteActor>& actors) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_LADDER };
    r.scale = size;
    r.ratio = 0;
    r.action = 6;
    actors.push_back(r);
  }
};

DECLARE_ENTITY("ladder", Ladder);
}

