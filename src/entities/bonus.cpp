// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Bonus entity

#include <cmath> // sin
#include <algorithm>

#include "base/util.h"
#include "base/scene.h"

#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "entities/player.h"

struct Bonus : Entity
{
  Bonus(int modelAction_, int type_, char const* msg_)
  {
    modelAction = modelAction_;
    type = type_;
    msg = msg_;
    size = UnitSize;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto s = sin(time * 0.01);
    auto r = Actor { pos, MDL_BONUS };
    r.scale = UnitSize;
    r.ratio = max(s, 0.0);
    r.action = modelAction;

    actors.push_back(r);
  }

  virtual void tick() override
  {
    ++time;
  }

  void onCollide(Body* other)
  {
    if(dead)
      return;

    if(auto player = dynamic_cast<Player*>(other))
    {
      player->addUpgrade(type);
      game->playSound(SND_BONUS);
      game->textBox(msg);
      dead = true;
    }
  }

  int time = 0;
  int modelAction;
  int type;
  char const* msg;
};

std::unique_ptr<Entity> makeBonus(int action, int upgradeType, char const* msg)
{
  return make_unique<Bonus>(action, upgradeType, msg);
}

