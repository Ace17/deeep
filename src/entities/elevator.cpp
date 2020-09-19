// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <cmath> // sin

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h"
#include "gameplay/move.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h"

struct Elevator : Entity
{
  Elevator()
  {
    solid = true;
    pusher = true;
    size = Size(2, 1);
    collisionGroup = CG_WALLS;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void enter() override
  {
    pos.x += 0.001;
    size.width -= 0.002;
    initialPos = pos;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_ELEVATOR };
    r.scale = size;
    r.ratio = 0;
    r.action = liftTimer > 0 ? 1 : 0;

    if(liftTimer > 0)
      r.effect = Effect::Blinking;

    actors.push_back(r);
  }

  void onCollide(Body* other)
  {
    if(other->pos.y > pos.y + size.height / 2)
    {
      if(debounceTrigger == 0)
        trigger();

      debounceTrigger = 100;
    }
  }

  void trigger()
  {
    if(liftTimer > 0)
      return;

    game->playSound(SND_SWITCH);
    liftTimer = 5000;
  }

  void tick() override
  {
    for(int i = 0; i < 10; ++i)
      subTick();
  }

  void subTick()
  {
    decrement(liftTimer);
    decrement(debounceTrigger);

    auto liftVel = Vector(0, 0.00466);

    if(liftTimer >= 4500)
    {
      // initial pause
    }
    else if(liftTimer >= 3000)
    {
      // go up
      physics->moveBody(this, liftVel);
    }
    else if(liftTimer >= 2000)
    {
      // pause in top position
      auto topPos = initialPos + Vector(0, 7);
      physics->moveBody(this, topPos - pos);
    }
    else if(liftTimer > 500)
    {
      // go down
      physics->moveBody(this, -1.0 * liftVel);
    }
    else if(liftTimer < 500)
    {
      // stuck in initial position
      physics->moveBody(this, initialPos - pos);
    }
  }

  int liftTimer = 0;
  int debounceTrigger = 0;

  Vector initialPos;
};

#include "gameplay/entity_factory.h"

static auto const reg2 = registerEntity("elevator", [] (IEntityConfig*)  -> unique_ptr<Entity> { return make_unique<Elevator>(); });

