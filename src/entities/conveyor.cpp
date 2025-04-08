// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"

#include <cmath>

namespace
{
struct Conveyor : Entity
{
  Conveyor(IEntityConfig* cfg)
  {
    size.x = cfg->getInt("width", 1);
    size.y = cfg->getInt("height", 1);
    speed = cfg->getInt("speed", 30) / 1000.0f;
    collisionGroup = CG_WALLS;
    collidesWith = CG_PLAYER;
    solid = 1;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void tick() override
  {
    m_time += fabs(speed);

    if(m_time >= 1.0f)
      m_time -= 1.0f;
  }

  void addActors(std::vector<SpriteActor> &) const override {};

  void addActors(std::vector<TileActor>& actors) const override
  {
    const Rect2f rect { pos, size };
    auto r = TileActor { rect, MDL_RECT };
    r.ratio = fmod(m_time, 1);

    if(speed < 0)
      r.ratio = 1 - r.ratio;

    r.action = 2;
    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    // avoid infinite recursion
    // (if the conveyor pushes the player towards the conveyor)
    if(noRecurse)
      return;

    noRecurse = true;
    physics->moveBody(other, Vector(speed, 0));
    noRecurse = false;
  }

  float speed = 0;
  bool noRecurse = false;
  float m_time = 0;
};

DECLARE_ENTITY("conveyor", Conveyor);
}

