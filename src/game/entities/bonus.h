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

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"

struct Bonus : Entity
{
  Bonus()
  {
    size = Size2f(0.5, 0.5);
  }

  virtual Actor getActor() const override
  {
    auto s = sin(time * 0.01);
    auto r = Actor(pos, MDL_BONUS);
    r.scale = Vector2f(1, 1);
    r.ratio = max(s, 0.0);
    r.action = 0;

    return r;
  }

  virtual void tick() override
  {
    ++time;
  }

  virtual void onCollide(Entity*) override
  {
    if(dead)
      return;

    game->playSound(SND_BONUS);
    dead = true;
  }

  int time;
};

