// Copyright (C) 2025 - Sebastien Alaiwan
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

#include <cmath>

namespace
{
struct Water : Entity, Swimmable
{
  Water(IEntityConfig* cfg)
  {
    size.x = cfg->getInt("width", 1);
    size.y = cfg->getInt("height", 1);
    solid = 0;
    collisionGroup = CG_LADDER;
  }

  void tick() override
  {
    m_time += 0.007;

    if(m_time >= 1.0f)
      m_time -= 1.0f;
  }

  void addActors(IActorSink* sink) const override
  {
    const Rect2f rect { pos, size };
    auto r = TileActor { rect, MDL_WATER };
    r.ratio = fmod(m_time, 1);
    r.zOrder = 2;
    sink->sendActor(r);
  }

  float m_time = 0;
};

DECLARE_ENTITY("water", Water);
}

