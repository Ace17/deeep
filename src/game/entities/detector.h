/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/toggle.h"

struct TouchDetectorEvent : Event
{
  TouchDetectorEvent(int whichOne_)
  {
    whichOne = whichOne_;
  }

  int whichOne;
};

struct Detector : public Entity
{
  Detector()
  {
    size = Size2f(0.1, 3);
    solid = false;
    collisionGroup = 0; // dont' trigger other detectors
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = Vector2f(size.width, size.height);
    return r;
  }

  virtual void tick() override
  {
    decrement(touchDelay);
  }

  virtual void onCollide(Entity*) override
  {
    if(touchDelay)
      return;

    game->playSound(SND_SWITCH);
    game->postEvent(make_unique<TouchDetectorEvent>(id));
    touchDelay = 1000;
  }

  Int id;
  Int touchDelay;
};

