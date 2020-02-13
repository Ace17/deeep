// Copyright (C) 2021 - Sebastien Alaiwan
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
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"
#include "gameplay/player.h"
#include "gameplay/sounds.h"

namespace
{
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
    auto r = Actor { id, pos, MDL_BONUS };
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

template<int action_, int upgradeType_, const char msg_[]>
struct ConcreteBonus : Bonus
{
  ConcreteBonus(IEntityConfig*) : Bonus(action_, upgradeType_, msg_) {}
};

const char UpgradeShootMsg[] = "press Z";
using UpgradeBonus_Shoot = ConcreteBonus<3, UPGRADE_SHOOT, UpgradeShootMsg>;
DECLARE_ENTITY("upgrade_shoot", UpgradeBonus_Shoot);

const char UpgradeClimbMsg[] = "jump while against wall";
using UpgradeBonus_Climb = ConcreteBonus<4, UPGRADE_CLIMB, UpgradeClimbMsg>;
DECLARE_ENTITY("upgrade_climb", UpgradeBonus_Climb);

const char UpgradeDashMsg[] = "press C";
using UpgradeBonus_Dash = ConcreteBonus<5, UPGRADE_DASH, UpgradeDashMsg>;
DECLARE_ENTITY("upgrade_dash", UpgradeBonus_Dash);

const char UpgradeDjumpMsg[] = "double jump";
using UpgradeBonus_Djump = ConcreteBonus<6, UPGRADE_DJUMP, UpgradeDjumpMsg>;
DECLARE_ENTITY("upgrade_djump", UpgradeBonus_Djump);

const char UpgradeBallMsg[] = "press down";
using UpgradeBonus_Ball = ConcreteBonus<7, UPGRADE_BALL, UpgradeBallMsg>;
DECLARE_ENTITY("upgrade_ball", UpgradeBonus_Ball);

const char UpgradeSlideMsg[] = "go against wall while falling";
using UpgradeBonus_Slide = ConcreteBonus<8, UPGRADE_SLIDE, UpgradeSlideMsg>;
DECLARE_ENTITY("upgrade_slide", UpgradeBonus_Slide);

const char UpgradeBombMsg[] = "press Z in ball mode";
using UpgradeBonus_Bomb = ConcreteBonus<9, UPGRADE_BOMB, UpgradeBombMsg>;
DECLARE_ENTITY("upgrade_bomb", UpgradeBonus_Bomb);

const char UpgradeLifeMsg[] = "life up";
using UpgradeBonus_Life = ConcreteBonus<0, 0, UpgradeLifeMsg>;
DECLARE_ENTITY("bonus_life", UpgradeBonus_Life);
}

std::unique_ptr<Entity> makeBonus(int action, int upgradeType, char const* msg)
{
  return make_unique<Bonus>(action, upgradeType, msg);
}

