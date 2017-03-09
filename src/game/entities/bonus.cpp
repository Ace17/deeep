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

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/entities/player.h"

struct Bonus : Entity
{
  Bonus(int modelAction_, int type_)
  {
    modelAction = modelAction_;
    type = type_;
    size = Size2f(1, 1);
  }

  virtual Actor getActor() const override
  {
    auto s = sin(time * 0.01);
    auto r = Actor(pos, MDL_BONUS);
    r.scale = Size2f(1, 1);
    r.ratio = max(s, 0.0);
    r.action = modelAction;

    return r;
  }

  virtual void tick() override
  {
    ++time;
  }

  virtual void onCollide(Entity* other) override
  {
    if(dead)
      return;

    if(auto player = dynamic_cast<Player*>(other))
    {
      player->addUpgrade(type);
      game->playSound(SND_BONUS);
      dead = true;
    }
  }

  int time = 0;
  int modelAction;
  int type;
};

std::unique_ptr<Entity> makeBonus(int action, int upgradeType)
{
  return make_unique<Bonus>(action, upgradeType);
}

