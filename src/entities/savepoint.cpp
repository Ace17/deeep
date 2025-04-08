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
#include "gameplay/models.h" // MDL_BLOCK
#include "gameplay/player.h"
#include "gameplay/sounds.h" // SND_HATCH
#include "gameplay/toggle.h"
#include "gameplay/vec.h"

#include <cmath>

namespace
{
struct SavePoint : Entity
{
  SavePoint(IEntityConfig*)
  {
    solid = 0;
    size = UnitSize;
    collisionGroup = 0;
    collidesWith = CG_PLAYER | CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(std::vector<SpriteActor>& actors) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_SAVEPOINT };
    r.scale = size;
    r.action = 3;
    r.ratio = fmod(m_time, 1);

    if(timer)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  void enter() override
  {
    timer = 10;
  }

  void tick() override
  {
    decrement(timer);

    m_time += 0.01;

    if(m_time >= 1.0f)
      m_time -= 1.0f;
  }

  void onCollide(Body* other)
  {
    if(dynamic_cast<Playerable*>(other))
    {
      if(timer == 0)
      {
        game->playSound(SND_SAVEPOINT);
        game->postEvent(std::make_unique<SaveEvent>());
        game->textBox("Game Saved");
      }

      timer = 150;
    }
  }

  static constexpr uint32_t flags = EntityFlag_ShowOnMinimap_S;

  int timer = 0;
  float m_time = 0;
};

DECLARE_ENTITY("savepoint", SavePoint);
}

