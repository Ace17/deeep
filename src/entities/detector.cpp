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
#include "gameplay/sounds.h"
#include "gameplay/toggle.h"

namespace
{
struct RoomBoundaryDetector : Entity
{
  RoomBoundaryDetector(IEntityConfig* cfg)
  {
    targetLevel = cfg->getInt("target_level");
    transform = Vector(cfg->getInt("transform_x"), cfg->getInt("transform_y"));
    size = UnitSize * 16;
    solid = false;
    collisionGroup = 0;
    collidesWith = CG_PLAYER | CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_RECT };
    r.scale = size;
    actors.push_back(r);
  }

  void onCollide(Body*)
  {
    if(touched)
      return;

    game->postEvent(make_unique<TouchLevelBoundary>(targetLevel, transform));
    touched = true;
  }

  int targetLevel = 0;
  Vector transform;
  bool touched = false;
};

struct RoomBoundaryBlocker : Entity
{
  RoomBoundaryBlocker(const IEntityConfig*)
  {
    size = UnitSize * 16;
    solid = true;
    collisionGroup = CG_WALLS;
    collidesWith = -1;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_RECT };
    r.scale = size;
    r.effect = Effect::Blinking;
    actors.push_back(r);
  }
};

DECLARE_ENTITY("room_boundary_detector", RoomBoundaryDetector);
DECLARE_ENTITY("blocker", RoomBoundaryBlocker);
}

