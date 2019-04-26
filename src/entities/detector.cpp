// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/util.h"
#include "base/scene.h"

#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "toggle.h"
#include "collision_groups.h"

struct RoomBoundaryDetector : Entity
{
  RoomBoundaryDetector(int targetLevel_, Vector transform_)
  {
    targetLevel = targetLevel_;
    transform = transform_;
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
  RoomBoundaryBlocker(int groupsToBlock)
  {
    size = UnitSize * 16;
    solid = true;
    collisionGroup = CG_WALLS;
    collidesWith = groupsToBlock;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_RECT };
    r.scale = size;
    r.effect = Effect::Blinking;
    actors.push_back(r);
  }
};

#include "entity_factory.h"

static auto makeBoundaryDetector(IEntityConfig* cfg)
{
  auto targetLevel = cfg->getInt("0");
  auto transform = Vector(cfg->getInt("1"), cfg->getInt("2"));
  return make_unique<RoomBoundaryDetector>(targetLevel, transform);
}

static auto const reg1 = registerEntity("room_boundary_detector", &makeBoundaryDetector);
static auto const reg2 = registerEntity("blocker", [] (IEntityConfig*) { return make_unique<RoomBoundaryBlocker>(-1); });

