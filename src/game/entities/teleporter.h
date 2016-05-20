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

class Teleporter : public Entity
{
public:
  Teleporter()
  {
    size = Size2f(2, 1);
    solid = true;
    state = false;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_TELEPORTER);
    r.scale = Vector2f(2, 2);
    r.pos -= Vector2f(1, 0);
    r.ratio = state ? 1 : 0;
    return r;
  }

  virtual void tick() override
  {
  }

  virtual void onCollide(Entity* other) override
  {
    if(state)
      return;

    if(abs(other->vel.x) >= 0.001)
      return;

    state = true;
    game->playSound(SND_SWITCH);
    // game->trigger(TRG_ENDLEVEL);
  }

  bool state;
};

