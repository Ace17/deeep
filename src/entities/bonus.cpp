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

  void enter() override
  {
    auto var = game->getVariable(id);

    // already picked up?
    if(var->get())
      dead = true;
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

      auto var = game->getVariable(id);
      var->set(1);
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

#include "entity_factory.h"
static auto const reg1 = registerEntity("upgrade_climb", [] (EntityConfig &) { return makeBonus(4, UPGRADE_CLIMB, "jump while against wall"); });
static auto const reg2 = registerEntity("upgrade_shoot", [] (EntityConfig &) { return makeBonus(3, UPGRADE_SHOOT, "press Z"); });
static auto const reg3 = registerEntity("upgrade_dash", [] (EntityConfig &) { return makeBonus(5, UPGRADE_DASH, "press C while walking"); });
static auto const reg4 = registerEntity("upgrade_djump", [] (EntityConfig &) { return makeBonus(6, UPGRADE_DJUMP, "jump while airborne"); });
static auto const reg5 = registerEntity("upgrade_ball", [] (EntityConfig &) { return makeBonus(7, UPGRADE_BALL, "press down"); });
static auto const reg6 = registerEntity("upgrade_slide", [] (EntityConfig &) { return makeBonus(8, UPGRADE_SLIDE, "go against wall while falling"); });
static auto const reg7 = registerEntity("bonus_life", [] (EntityConfig &) { return makeBonus(0, 0, "life up"); });

