/**
 * @brief Exit portal.
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

class Teleporter : public Entity
{
public:
  Teleporter()
  {
    size = Size2f(2, 0.5);
    solid = false;
    state = false;
    collisionGroup = -1;
    collidesWith = -1;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_TELEPORTER);
    r.pos.y -= size.height * 0.9;
    r.scale = Vector2f(size.width, size.height * 4);
    r.ratio = state ? 1 : 0;

    if(blink)
      r.effect = EFFECT_BLINKING;

    return r;
  }

  virtual void tick() override
  {
    decrement(blink);

    if(target)
    {
      blink = 100;
      target->pos.x = pos.x + 0.5;
      target->vel.x = 0;
      target->vel.y = 0;

      decrement(delay);

      if(delay == 0)
      {
        target = nullptr;
        dead = true;

        {
          auto evt = make_unique<TriggerEvent>();
          evt->idx = -1;
          game->postEvent(move(evt));
        }
      }
    }
  }

  virtual void onCollide(Entity* other) override
  {
    if(state)
      return;

    blink = 100;

    if(abs(other->pos.x - pos.x - 0.5) >= 0.01)
      return;

    state = true;
    target = other;
    game->playSound(SND_TELEPORT);
    delay = 1000;
  }

  bool state;
  Int blink;
  Int delay;
  Entity* target = nullptr;
};

