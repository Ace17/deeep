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

#include "explosion.h"

namespace
{
struct Sweeper : Entity, Damageable
{
  Sweeper(IEntityConfig*)
  {
    size = Size(0.8, 0.8);
    collisionGroup = CG_WALLS;
    collisionGroup = CG_ENEMIES;
    collidesWith = CG_WALLS | CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };

    vel.x = 0.03;
    vel.y = 0.03;
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_SWEEPER };

    r.scale = size;
    r.pos += Vector(-(r.scale.x - size.x) * 0.5, 0);

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = 0;
    r.ratio = (time % 80) / 80.0f;

    sink->sendActor(r);
  }

  void tick() override
  {
    ++time;

    auto trace = slideMove(this, vel);

    if(!trace.horz)
      vel.x = -vel.x;

    if(!trace.vert)
      vel.y = -vel.y;

    decrement(blinking);
  }

  void onCollide(Body* other)
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(5);
  }

  void onDamage(int amount) override
  {
    blinking = 75;
    life -= amount;

    game->playSound(SND_DAMAGE);

    if(life < 0)
    {
      game->playSound(SND_EXPLODE);
      dead = true;

      auto explosion = makeExplosion();
      explosion->pos = getCenter();
      game->spawn(explosion.release());
    }
  }

  int life = 30;
  int time = 0;
  Vector vel;
};

DECLARE_ENTITY("sweeper", Sweeper);
}

