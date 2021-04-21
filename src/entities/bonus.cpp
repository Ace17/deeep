// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Bonus entity

#include <cmath> // sin

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h"
#include "gameplay/player.h"
#include "gameplay/sounds.h"

struct Bonus : Entity
{
  Bonus(int modelAction_, int type_, char const* msg_)
  {
    modelAction = modelAction_;
    type = type_;
    msg = msg_;
    size = UnitSize;
    Body::onCollision = [this] (Body* other) { onCollide(other); };

    collidesWith = CG_SOLIDPLAYER;
    collisionGroup = CG_BONUS;
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
    auto s = sin(time * 0.1);
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

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("upgrade_climb", [] (IEntityConfig*) { return makeBonus(4, UPGRADE_CLIMB, "jump while against wall"); });
static auto const reg2 = registerEntity("upgrade_shoot", [] (IEntityConfig*) { return makeBonus(3, UPGRADE_SHOOT, "press Z"); });
static auto const reg3 = registerEntity("upgrade_dash", [] (IEntityConfig*) { return makeBonus(5, UPGRADE_DASH, "press C while walking"); });
static auto const reg4 = registerEntity("upgrade_djump", [] (IEntityConfig*) { return makeBonus(6, UPGRADE_DJUMP, "jump while airborne"); });
static auto const reg5 = registerEntity("upgrade_ball", [] (IEntityConfig*) { return makeBonus(7, UPGRADE_BALL, "press down"); });
static auto const reg6 = registerEntity("upgrade_slide", [] (IEntityConfig*) { return makeBonus(8, UPGRADE_SLIDE, "go against wall while falling"); });
static auto const reg7 = registerEntity("bonus_life", [] (IEntityConfig*) { return makeBonus(0, 0, "life up"); });

