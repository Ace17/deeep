/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "base/scene.h" // Actor

#include "collision_groups.h"
#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "toggle.h" // decrement

#include "explosion.h"

struct SpiderBullet : Entity
{
  SpiderBullet()
  {
    size = Size(0.3, 0.3);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = size;
    r.action = 0;
    r.ratio = 0;

    return r;
  }

  void tick() override
  {
    pos += vel;
    decrement(life);

    if(life == 0)
      dead = true;
  }

  void onCollide(Entity* other) override
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(4);

    dead = true;
  }

  int life = 1000;
};

struct Spider : Entity, Damageable
{
  Spider()
  {
    dir = -1.0f;
    size = Size(1, 1);
    collisionGroup = CG_WALLS;
    collidesWith = CG_SOLIDPLAYER;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SPIDER);

    r.scale = Size(1, 1);

    if(blinking)
      r.effect = Effect::Blinking;

    r.action = 0;
    r.ratio = (time % 200) / 200.0f;

    if(dir > 0)
      r.scale.width = -r.scale.width;

    return r;
  }

  virtual void tick() override
  {
    ++time;

    vel.x = vel.y = 0;

    decrement(blinking);

    if(time % 1500 == 0)
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
    auto const speed = 0.01;
    auto bullet = make_unique<SpiderBullet>();
    bullet->pos = getCenter();
    bullet->vel = Vector(cos(angle) * speed, sin(angle) * speed);
    game->spawn(bullet.release());
  }

  virtual void onCollide(Entity* other) override
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(5);
  }

  virtual void onDamage(int amount) override
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

unique_ptr<Entity> makeSpider()
{
  return make_unique<Spider>();
}

