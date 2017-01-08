#pragma once

#include <algorithm>

#include "base/util.h"
#include "base/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/entities/explosion.h"

struct SpiderBullet : Entity
{
  SpiderBullet()
  {
    size = Size2f(0.3, 0.3);
    collisionGroup = (1 << 1);
    collidesWith = 1; // only the player
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);
    r.scale = Vector2f(size.width, size.height);
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
    other->onDamage(4);
    dead = true;
  }

  int life = 1000;
};

struct Spider : Entity
{
  Spider()
  {
    dir = -1.0f;
    size = Size2f(1, 1);
    collisionGroup = (1 << 1);
    collidesWith = 1; // only the player
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_RECT);

    r.scale = Vector2f(1, 1);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    r.action = 0;
    r.ratio = (time % 800) / 800.0f;

    if(dir > 0)
      r.scale.x = -r.scale.x;

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
    bullet->vel = Vector2f(cos(angle) * speed, sin(angle) * speed);
    game->spawn(bullet.release());
  }

  virtual void onCollide(Entity* other) override
  {
    other->onDamage(5);
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

  Int life = 60;
  Int time;
  float dir;
};

