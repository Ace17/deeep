// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"
#include "gameplay/move.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h"
#include "misc/math.h"

namespace
{
struct Lift : Entity
{
  Lift(IEntityConfig* cfg)
  {
    solid = true;
    pusher = true;
    size = Size(2, 1);
    unstable = cfg->getInt("unstable", 0);
    link = cfg->getInt("link", 0);
    delta_x = cfg->getInt("delta_x", 0);
    delta_y = cfg->getInt("delta_y", +7);
    collisionGroup = CG_WALLS;
    collidesWith = CG_PLAYER;
  }

  void enter() override
  {
    initialPos = pos;
    finalPos = pos + Vector(delta_x, delta_y);
    moveDir = normalize(finalPos - initialPos);

    if(link)
    {
      auto onTriggered = [&] (int) { trigger(); };
      auto var = game->getVariable(link);
      subscription = var->observe(onTriggered);
    }
    else // activate by touch
    {
      Body::onCollision = [this] (Body* other) { onCollide(other); };
    }
  }

  void leave() override
  {
    subscription.reset();
  }

  void onCollide(Body* other)
  {
    if(other->pos.y > pos.y + size.y / 2)
    {
      if(debounceTrigger == 0)
        trigger();

      debounceTrigger = 10;
    }
  }

  void addActors(std::vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_ELEVATOR };
    r.scale = size;
    r.ratio = 0;
    r.action = state ? 1 : 0;

    if(state && !unstable)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  void trigger()
  {
    if(state)
      return;

    game->playSound(SND_SWITCH);
    timer = 50;
    state = 1;
  }

  void tick() override
  {
    decrement(debounceTrigger);

    const auto liftSpeed = 0.05;
    switch(state)
    {
    case 0: // at rest
      physics->moveBody(this, initialPos - pos); // stick to initial pos
      timer = 50;

      if(unstable)
        state = 1;

      break;

    case 1: // initial pause

      if(decrement(timer))
        state = 2;

      break;

    case 2: // moving: initialPos -> finalPos

      if(dotProduct(pos - finalPos, moveDir) < -0.1)
      {
        // go forward
        physics->moveBody(this, liftSpeed * moveDir);
      }
      else
      {
        state = 3;
        timer = 50;
      }

      break;

    case 3: // pause in top position
      physics->moveBody(this, finalPos - pos);

      if(decrement(timer))
        state = 4;

      break;

    case 4: // moving: finalPos -> initialPos

      if(dotProduct(pos - initialPos, moveDir) > 0.1)
      {
        // go backwards
        physics->moveBody(this, -1.0 * liftSpeed * moveDir);
      }
      else
      {
        state = 0;
      }

      break;
    }
  }

  int state = 0;
  int timer = 0;
  int debounceTrigger = 0;

  // config
  int unstable = 0;
  int delta_x = 0;
  int delta_y = 0;
  int link = 0;

  std::unique_ptr<Handle> subscription;
  Vector initialPos;
  Vector finalPos;
  Vector moveDir; // normalized
};

struct LegacyElevator : Lift
{
  LegacyElevator(IEntityConfig* cfg) : Lift(cfg)
  {
    unstable = 0;
    link = 0;
    delta_x = 0;
    delta_y = 7;
  }
};
}

DECLARE_ENTITY("lift", Lift);
DECLARE_ENTITY("elevator", LegacyElevator);

