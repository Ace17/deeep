// Copyright (C) 2019 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"
#include "base/util.h"

#include "collision_groups.h"
#include "entities/move.h"
#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "toggle.h"

#include "entity_factory.h"
#include <cmath> // sqrt

namespace
{
float dotProduct(Vector a, Vector b)
{
  return a.x * b.x + a.y * b.y;
}

Vector normalize(Vector a)
{
  auto const magnitude = sqrt(dotProduct(a, a));
  return a * 1.0 / magnitude;
}

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
  }

  void enter() override
  {
    pos.x += 0.001;
    size.width -= 0.002;
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
    if(other->pos.y > pos.y + size.height / 2)
    {
      if(debounceTrigger == 0)
        trigger();

      debounceTrigger = 10;
    }
  }

  virtual void addActors(vector<Actor>& actors) const override
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

  unique_ptr<Handle> subscription;
  Vector initialPos;
  Vector finalPos;
  Vector moveDir; // normalized
};
}

DECLARE_ENTITY("lift", Lift);

