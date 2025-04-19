// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Player-controlled switch

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
struct Switch : Entity
{
  Switch(IEntityConfig* cfg)
    : id(cfg->getInt("link", cfg->getInt("0")))
  {
    size = UnitSize * 0.75;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
    collisionGroup = CG_DOORS;
    collidesWith = CG_PLAYER;
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + UnitSize / 2, MDL_SWITCH };
    r.scale = UnitSize;

    if(blinking)
      r.effect = Effect::Blinking;

    auto var = game->getVariable(id);
    r.action = var->get() ? 1 : 0;

    sink->sendActor(r);
  }

  void tick() override
  {
    blinking = std::max(0, blinking - 1);
  }

  void onCollide(Body*)
  {
    if(blinking)
      return;

    blinking = 200;
    game->playSound(SND_SWITCH);

    auto var = game->getVariable(id);
    const bool newState = !var->get();
    var->set(newState);
  }

  const int id;
};

DECLARE_ENTITY("switch", Switch);
}

