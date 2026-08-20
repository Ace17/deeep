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
#include "misc/math.h"
#include <cmath>

namespace
{
struct TrackPoint : Entity
{
  TrackPoint(IEntityConfig* cfg)
    : nextId(cfg->getInt("next", 0))
  {
  }

  const int nextId;

  void addActors(IActorSink*) const override {}
};

struct TrackedLift : Entity
{
  TrackedLift(IEntityConfig* cfg)
  {
    solid = true;
    pusher = true;
    size = Size(2, 1);

    trackId = cfg->getInt("track", 0);
    speed = cfg->getInt("speed", 50) / 1000.0f;

    collisionGroup = CG_MOVINGWALLS;
    collidesWith = CG_PLAYER;
  }

  void enter() override
  {
    auto currId = trackId;
    waypoints.clear();

    while(currId > 0)
    {
      auto trackPoint = dynamic_cast<const TrackPoint*>(game->getEntityById(currId));
      waypoints.push_back(trackPoint->pos);

      currId = trackPoint->nextId;

      if(currId == trackId)
        break;
    }
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_ELEVATOR };
    r.scale = size;
    r.ratio = 0;
    r.action = 1;
    sink->sendActor(r);
  }

  void tick() override
  {
    const auto delta = waypoints[targetPoint] - pos;
    const auto dist = sqrt(dotProduct(delta, delta));
    const auto dir = delta / dist;

    if(dist < 0.2)
    {
      targetPoint++;
      targetPoint %= waypoints.size();
      physics->moveBody(this, delta);
    }
    else
    {
      physics->moveBody(this, dir * speed);
    }
  }

  // config
  int trackId = 0;
  int targetPoint = 0;
  float speed = 0;
  std::vector<Vector> waypoints;
};
}

DECLARE_ENTITY("tracked_lift", TrackedLift);
DECLARE_ENTITY("track_point", TrackPoint);

