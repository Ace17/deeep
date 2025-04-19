// Copyright (C) 2025 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h" // decrement

#include "explosion.h"

namespace
{
struct DamageSensor : Entity, Damageable
{
  DamageSensor()
  {
    solid = true;
    collisionGroup = CG_WALLS;
    size = UnitSize;
  }

  void onDamage(int /*amount*/) override
  {
    onDamageDg();
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_SHUTTER };
    r.action = 0;
    r.ratio = 0;
    r.scale = size;
    sink->sendActor(r);
  }

  Delegate<void()> onDamageDg = [] () {};
};

struct Shutter : Entity
{
  Shutter(IEntityConfig* args) : link(args->getInt("link")), initialState(args->getInt("initial", 0))
  {
    size = Size(1, 4);
    collisionGroup = CG_DOORS | CG_WALLS;
  }

  void enter() override
  {
    auto onTriggered = [&] (int open)
      {
        toggle(open);
      };

    auto var = game->getVariable(link);
    subscription = var->observe(onTriggered);

    // already open?
    state = (var->get() + initialState) % 2;
    solid = !state;

    DamageSensor* sensor = new DamageSensor;
    sensor->onDamageDg = [this](){ if(delay == 0) toggle(!state); };
    sensor->pos = pos + Vec2f(-1, 3);
    game->spawn(sensor);
  }

  void leave() override
  {
    subscription.reset();
  }

  void tick() override
  {
    decrement(delay);

    if(delay == 0 && state)
      solid = false;
  }

  void toggle(int open)
  {
    state = (open + initialState) % 2;
    delay = 50;

    // in case of closing, immediately prevent traversal
    if(!state)
      solid = true;

    game->playSound(SND_DOOR);
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_SHUTTER };
    r.action = state ? 0 : 1;
    r.ratio = 1 - (delay / 50.0f);
    r.scale = size;
    sink->sendActor(r);
  }

  bool state = false;
  int delay = 0;
  const int link;
  const int initialState;
  std::unique_ptr<Handle> subscription;
};

DECLARE_ENTITY("shutter", Shutter);
}

