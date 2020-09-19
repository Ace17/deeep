// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/scene.h"
#include "base/util.h"

#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/models.h"
#include "gameplay/move.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h"

#include "explosion.h"

struct Sweeper : Entity, Damageable
{
  Sweeper()
  {
    size = Size(0.8, 0.8);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };

    vel.x = 0.03;
    vel.y = 0.03;
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_SWEEPER };

    r.scale = size;
    r.pos += Vector(-(r.scale.width - size.width) * 0.5, 0);

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = 0;
    r.ratio = (time % 80) / 80.0f;

    actors.push_back(r);
  }

  virtual void tick() override
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

  virtual void onDamage(int amount) override
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

#include "gameplay/entity_factory.h"
static auto const reg1 = registerEntity("sweeper", [] (IEntityConfig*)  -> unique_ptr<Entity> { return make_unique<Sweeper>(); });

