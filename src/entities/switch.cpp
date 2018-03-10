/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Switch and door.

#include <algorithm>
#include <cmath>

#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "toggle.h"

struct Switch : Entity
{
  Switch(int id_) : id(id_)
  {
    size = UnitSize * 0.75;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void enter() override
  {
    auto var = game->getVariable(id);
    state = var->get();
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor(pos, MDL_SWITCH);
    r.scale = size;

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = state ? 1 : 0;

    actors.push_back(r);
  }

  virtual void tick() override
  {
    blinking = max(0, blinking - 1);
  }

  void onCollide(Body*)
  {
    if(blinking)
      return;

    blinking = 2000;
    state = !state;
    game->playSound(SND_SWITCH);

    auto var = game->getVariable(id);
    var->set(state);
  }

  bool state = false;
  const int id;
};

unique_ptr<Entity> makeSwitch(int id)
{
  return make_unique<Switch>(id);
}

struct Door : Entity
{
  Door(int id_) : id(id_)
  {
    size = Size(1, 3);
    solid = true;
  }

  void enter() override
  {
    auto onTriggered = [&] (int open)
      {
        state = open;
        delay = 1000;

        // in case of closing, immediately prevent traversal
        if(!open)
          solid = true;
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
    auto r = Actor(pos, MDL_DOOR);
    r.action = state ? 1 : 3;
    r.ratio = 1 - (delay / 1000.0f);
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
    auto r = Actor(pos, MDL_DOOR);
    r.scale = size;

    if(blinking)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  virtual void tick() override
  {
    decrement(blinking);
  }

  virtual void onDamage(int amount) override
  {
    blinking = 200;
    life -= amount;

    if(life < 0)
    {
      game->playSound(SND_EXPLODE);
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

unique_ptr<Entity> makeBreakableDoor()
{
  return make_unique<BreakableDoor>();
}

