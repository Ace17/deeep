/**
 * @brief Switch and door.
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <algorithm>
#include <cmath>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/toggle.h"
#include "game/entities/explosion.h"

struct TriggerEvent : Event
{
  int idx;
};

struct Switch : Entity
{
  Switch(int id_) : id(id_)
  {
    size = Size2f(0.5, 0.5);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SWITCH);
    r.scale = Vector2f(0.5, 0.5);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    r.action = state ? 1 : 0;
    return r;
  }

  virtual void tick() override
  {
    blinking = max(0, blinking - 1);
  }

  virtual void onCollide(Entity*) override
  {
    if(blinking || state)
      return;

    blinking = 1200;
    state = !state;
    game->playSound(SND_SWITCH);

    auto evt = make_unique<TriggerEvent>();
    evt->idx = id;
    game->postEvent(move(evt));
  }

  Bool state;
  const int id;
};

struct Door : Entity, IEventSink
{
  Door(int id_, IGame* g) : id(id_)
  {
    game = g;
    game->subscribeForEvents(this);
    size = Size2f(1, 1);
    solid = true;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SWITCH);
    r.action = 2 + (state ? 1 : 0);
    return r;
  }

  virtual void notify(const Event* evt) override
  {
    if(auto trg = evt->as<TriggerEvent>())
    {
      if(trg->idx != id)
        return;

      state = !state;
      solid = !solid;
    }
  }

  Bool state;
  const int id;
};

struct BreakableDoor : Entity
{
  BreakableDoor()
  {
    size = Size2f(1, 3);
    solid = true;
    collisionGroup = (1 << 1);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_DOOR);
    r.scale = Vector2f(size.width, size.height);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    return r;
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

  Int life = 130;
};

