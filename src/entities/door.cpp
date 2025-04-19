// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Various doors

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h" // decrement

#include "explosion.h"

namespace
{
struct Door : Entity
{
  Door(IEntityConfig* args) : link(args->getInt("link", args->getInt("0"))), initialState(args->getInt("initial", 0))
  {
    size = Size(1, 3);
    collisionGroup = CG_DOORS;
  }

  void enter() override
  {
    auto onTriggered = [&] (int open)
      {
        state = (open + initialState) % 2;
        delay = 50;

        // in case of closing, immediately prevent traversal
        if(!state)
          solid = true;

        game->playSound(SND_DOOR);
      };

    auto var = game->getVariable(link);
    subscription = var->observe(onTriggered);

    // already open?
    state = (var->get() + initialState) % 2;
    solid = !state;
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

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_DOOR };
    r.action = state ? 1 : 3;
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

struct BreakableDoor : Entity, Damageable
{
  BreakableDoor(IEntityConfig*)
  {
    size = Size(1, 3);
    solid = true;
    collisionGroup = CG_WALLS;
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_DOOR };
    r.scale = size;

    if(blinking)
      r.effect = Effect::Blinking;

    sink->sendActor(r);
  }

  void enter() override
  {
    // already broken?
    if(game->getVariable(id)->get())
      dead = true;
  }

  void tick() override
  {
    decrement(blinking);
  }

  void onDamage(int amount) override
  {
    blinking = 20;
    life -= amount;

    if(life < 0)
    {
      game->playSound(SND_EXPLODE);
      game->getVariable(id)->set(1);
      dead = true;

      auto explosion = makeExplosion();
      explosion->pos = getCenter();
      game->spawn(explosion.release());
    }
    else
      game->playSound(SND_DAMAGE);
  }

  int life = 130;
};

DECLARE_ENTITY("fragile_door", BreakableDoor);
DECLARE_ENTITY("door", Door);
}

