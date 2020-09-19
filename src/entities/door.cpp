// Copyright (C) 2019 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Various doors

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h" // decrement

struct Door : Entity
{
  Door(int id_) : id(id_)
  {
    size = Size(1, 3);
    solid = true;
    collisionGroup = CG_DOORS;
  }

  void enter() override
  {
    auto onTriggered = [&] (int open)
      {
        state = open;
        delay = 100;

        // in case of closing, immediately prevent traversal
        if(!open)
          solid = true;

        game->playSound(SND_DOOR);
      };

    auto var = game->getVariable(id);
    subscription = var->observe(onTriggered);

    // already open?
    state = var->get();

    if(state)
      solid = false;
  }

  void leave() override
  {
    subscription.reset();
  }

  virtual void tick() override
  {
    decrement(delay);

    if(delay == 0 && state)
      solid = false;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_DOOR };
    r.action = state ? 1 : 3;
    r.ratio = 1 - (delay / 100.0f);
    r.scale = size;
    actors.push_back(r);
  }

  bool state = false;
  int delay = 0;
  const int id;
  unique_ptr<Handle> subscription;
};

unique_ptr<Entity> makeDoor(int id)
{
  return make_unique<Door>(id);
}

///////////////////////////////////////////////////////////////////////////////

#include "explosion.h"

struct BreakableDoor : Entity, Damageable
{
  BreakableDoor()
  {
    size = Size(1, 3);
    solid = true;
    collisionGroup = CG_WALLS;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_DOOR };
    r.scale = size;

    if(blinking)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  void enter() override
  {
    // already broken?
    if(game->getVariable(id)->get())
      dead = true;
  }

  virtual void tick() override
  {
    decrement(blinking);
  }

  virtual void onDamage(int amount) override
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

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("fragile_door", [] (IEntityConfig*) -> unique_ptr<Entity> { return make_unique<BreakableDoor>(); });
static auto const reg2 = registerEntity("door", [] (IEntityConfig* args) -> unique_ptr<Entity> { auto arg = args->getInt("0"); return makeDoor(arg); });

