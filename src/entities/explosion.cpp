// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "explosion.h"

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/entity.h"
#include "gameplay/models.h"
#include "gameplay/player.h"
#include "gameplay/sounds.h"

static auto const DURATION = 40;

struct Explosion : Entity
{
  Explosion()
  {
    size = UnitSize * 0.1;
  }

  void tick() override
  {
    time++;

    if(time >= DURATION)
    {
      time = DURATION;
      dead = true;
    }
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_EXPLOSION };

    r.ratio = time / (float)DURATION;
    r.scale = Size(3, 3);
    r.pos += Vector(-r.scale.x * 0.5, -r.scale.y * 0.5);

    sink->sendActor(r);
  }

  int time = 0;
};

std::unique_ptr<Entity> makeExplosion()
{
  return std::make_unique<Explosion>();
}

