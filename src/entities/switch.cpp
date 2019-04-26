// Copyright (C) 2017 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Player-controlled switch

#include "base/util.h"
#include "base/scene.h"

#include "collision_groups.h"
#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "toggle.h"

struct Switch : Entity
{
  Switch(int id_) : id(id_)
  {
    size = UnitSize * 0.75;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void enter() override
  {
    auto var = game->getVariable(id);
    state = var->get();
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_SWITCH };
    r.scale = size;

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = state ? 1 : 0;

    actors.push_back(r);
  }

  virtual void tick() override
  {
    blinking = max(0, blinking - 1);
  }

  void onCollide(Body*)
  {
    if(blinking)
      return;

    blinking = 2000;
    state = !state;
    game->playSound(SND_SWITCH);

    auto var = game->getVariable(id);
    var->set(state);
  }

  bool state = false;
  const int id;
};

unique_ptr<Entity> makeSwitch(int id)
{
  return make_unique<Switch>(id);
}

#include "entity_factory.h"
static auto const reg1 = registerEntity("switch", [] (EntityConfig& args) { auto arg = atoi(args.getString("0").c_str()); return makeSwitch(arg); });

