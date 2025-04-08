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
struct MovingWall : Entity
{
  MovingWall(IEntityConfig* cfg)
  {
    size.x = cfg->getInt("width", 1);
    size.y = cfg->getInt("height", 1);
    solid = true;
    pusher = true;
    link = cfg->getInt("link", 0);
    delta_x = cfg->getInt("delta_x", 0);
    delta_y = cfg->getInt("delta_y", 0);
    collisionGroup = CG_MOVINGWALLS;
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
  }

  void leave() override
  {
    subscription.reset();
  }

  void addActors(std::vector<SpriteActor> &) const override {};

  void addActors(std::vector<TileActor>& actors) const override
  {
    const Rect2f rect { pos, size };
    auto r = TileActor { rect, MDL_TILES_00 };
    r.action = 16;
    r.zOrder = 1;
    actors.push_back(r);
  }

  void trigger()
  {
    if(state)
      return;

    game->playSound(SND_RUMBLE1);
    timer = 20;
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
        game->playSound(SND_RUMBLE2);
        state = 3;
        timer = 50;
      }

      break;

    case 3: // pause in top position
      physics->moveBody(this, finalPos - pos);

      if(decrement(timer))
      {
        game->playSound(SND_RUMBLE1);
        state = 4;
      }

      break;

    case 4: // moving: finalPos -> initialPos

      if(dotProduct(pos - initialPos, moveDir) > 0.1)
      {
        // go backwards
        physics->moveBody(this, -1.0 * liftSpeed * moveDir);
      }
      else
      {
        game->playSound(SND_RUMBLE2);
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
}

DECLARE_ENTITY("moving_wall", MovingWall);

