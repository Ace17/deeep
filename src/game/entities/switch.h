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

class Switch : public Entity
{
public:
  Switch(int id_) : id(id_)
  {
    size = Size2f(0.2, 0.2);
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
    game->trigger(id);
  }

  Bool state;
  const int id;
};

class Door : public Entity, public ITriggerable
{
public:
  Door(int id_, IGame* g) : id(id_)
  {
    game = g;
    game->listen(id, this);
    size = Size2f(1, 1);
    solid = true;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SWITCH);
    r.pos -= Vector2f(0.5, 0.5);
    r.action = 2 + (state ? 1 : 0);
    return r;
  }

  virtual void trigger() override
  {
    state = !state;
    solid = !solid;
  }

  Bool state;
  const int id;
};

