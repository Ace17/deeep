// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <cmath> // sin

#include "base/scene.h" // SpriteActor
#include "gameplay/collision_groups.h"
#include "gameplay/entity.h"
#include "gameplay/entity_factory.h"
#include "gameplay/models.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h" // decrement

#include "explosion.h"

namespace
{
struct SpiderBullet : Entity
{
  SpiderBullet()
  {
    size = Size(0.3, 0.3);
    collisionGroup = CG_ENEMIES;
    collidesWith = CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + size / 2, MDL_RECT };
    r.scale = size;
    r.action = 0;
    r.ratio = 0;

    sink->sendActor(r);
  }

  void tick() override
  {
    pos += vel;
    decrement(life);

    if(life == 0)
      dead = true;
  }

  void onCollide(Body* other)
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(4);

    dead = true;
  }

  int life = 100;
  Vector vel;
};

struct Spider : Entity, Damageable
{
  Spider(IEntityConfig*)
  {
    dir = -1.0f;
    size = Size(1, 1);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(IActorSink* sink) const override
  {
    auto r = SpriteActor { pos + Vec2f(0.5, 0.05), MDL_SPIDER };

    r.scale = Size(1, 1);

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = 0;
    r.ratio = (time % 20) / 20.0f;

    if(dir > 0)
      r.scale.x = -r.scale.x;

    sink->sendActor(r);
  }

  void tick() override
  {
    ++time;

    decrement(blinking);

    if(time % 150 == 0)
    {
      auto target = game->getPlayerPosition();
      auto delta = target - pos;

      if(delta.x * delta.x + delta.y * delta.y < 1000)
      {
        float base = 0;

        if(delta.x < 0)
          base = 3.14;

        shoot(base);
        shoot(base - 0.5);
        shoot(base + 0.5);
      }
    }
  }

  void shoot(float angle)
  {
    auto const speed = 0.1;
    auto bullet = std::make_unique<SpiderBullet>();
    bullet->pos = getCenter();
    bullet->vel = Vector(cos(angle) * speed, sin(angle) * speed);
    game->spawn(bullet.release());
  }

  void onCollide(Body* other)
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(5);
  }

  void onDamage(int amount) override
  {
    blinking = 100;
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

  int life = 60;
  int time = 0;
  float dir;
};

DECLARE_ENTITY("spider", Spider);
}

