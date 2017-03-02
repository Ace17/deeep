/**
 * @author Sebastien Alaiwan
 * @date 2015-11-16
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <algorithm>

#include "base/scene.h"
#include "base/util.h"

#include "game/entities/player.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/toggle.h"
#include "rockman.h"

auto const WALK_SPEED = 0.0075f;
auto const MAX_HORZ_SPEED = 0.02f;
auto const MAX_FALL_SPEED = 0.02f;
auto const CLIMB_DELAY = 100;
auto const HURT_DELAY = 500;

enum ORIENTATION
{
  LEFT,
  RIGHT,
};

struct Bullet : Entity
{
  Bullet()
  {
    size = Size2f(0.5, 0.4);
    collisionGroup = 0;
    collidesWith = (1 << 1);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_BULLET);
    r.scale = Vector2f(size.width, size.height);
    r.action = 0;
    r.ratio = 0;

    // re-center
    r.pos += Vector2f(r.scale.x * 0.5, 0);

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
    other->onDamage(10);
    dead = true;
  }

  int life = 1000;
};

struct Rockman : Player
{
  Rockman() : dir(RIGHT)
  {
    size = Size2f(0.9, 1.9);
    life = 31;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_ROCKMAN);
    r.scale = Vector2f(3, 3);

    // re-center
    r.pos += Vector2f(-(r.scale.x - size.width) * 0.5, -0.1);

    if(hurtDelay || life < 0)
    {
      r.action = ACTION_HURT;
      r.ratio = 1.0f - hurtDelay / float(HURT_DELAY);
    }
    else
    {
      if(!ground)
      {
        if(climbDelay)
        {
          r.action = ACTION_CLIMB;
          r.ratio = 1.0f - climbDelay / float(CLIMB_DELAY);
          r.scale.x *= -1;
        }
        else
        {
          r.action = ACTION_FALL;
          r.ratio = vel.y > 0 ? 0 : 1;
        }
      }
      else
      {
        if(vel.x != 0)
        {
          if(dashDelay)
          {
            r.ratio = min(400 - dashDelay, 400) / 100.0f;
            r.action = ACTION_DASH;
          }
          else
          {
            r.ratio = (time % 500) / 500.0f;

            if(shootDelay == 0)
              r.action = ACTION_WALK;
            else
              r.action = ACTION_WALK_SHOOT;
          }
        }
        else
        {
          if(shootDelay == 0)
          {
            r.ratio = (time % 1000) / 1000.0f;
            r.action = ACTION_STAND;
          }
          else
          {
            r.ratio = 0;
            r.action = ACTION_STAND_SHOOT;
          }
        }
      }
    }

    if(dir == LEFT)
      r.scale.x *= -1;

    if(blinking)
      r.effect = EFFECT_BLINKING;

    return r;
  }

  void think(Control const& c) override
  {
    control = c;
  }

  float health() override
  {
    return clamp(life / 31.0f, 0.0f, 1.0f);
  }

  virtual void addUpgrade(Int upgrade) override
  {
    upgrades |= upgrade;
    blinking = 2000;
    life = 31;
  }

  virtual int getUpgrades() override
  {
    return upgrades;
  }

  void computeVelocity(Control c)
  {
    if(hurtDelay || life <= 0)
      c = Control();

    airMove(c);

    if(ground)
      doubleJumped = false;

    if(vel.x > 0)
      dir = RIGHT;

    if(vel.x < 0)
      dir = LEFT;

    // gravity
    vel.y -= 0.00005;

    if(jumpbutton.toggle(c.jump))
    {
      if(ground)
      {
        game->playSound(SND_JUMP);
        vel.y = 0.015;
        doubleJumped = false;
      }
      else if(facingWall() && (upgrades & UPGRADE_CLIMB))
      {
        game->playSound(SND_JUMP);
        // wall climbing
        vel.x = dir == RIGHT ? -0.04 : 0.04;
        vel.y = 0.015;
        climbDelay = CLIMB_DELAY;
        doubleJumped = false;
      }
      else if((upgrades & UPGRADE_DJUMP) && !doubleJumped)
      {
        game->playSound(SND_JUMP);
        vel.y = 0.015;
        doubleJumped = true;
      }
    }

    // stop jump if the player release the button early
    if(vel.y > 0 && !c.jump)
      vel.y = 0;

    vel.x = clamp(vel.x, -MAX_HORZ_SPEED, MAX_HORZ_SPEED);
    vel.y = max(vel.y, -MAX_FALL_SPEED);
  }

  void airMove(Control c)
  {
    float wantedSpeed = 0;

    if(!climbDelay)
    {
      if(c.left)
        wantedSpeed -= WALK_SPEED;

      if(c.right)
        wantedSpeed += WALK_SPEED;
    }

    if(upgrades & UPGRADE_DASH)
    {
      if(dashbutton.toggle(c.dash) && ground && dashDelay == 0)
      {
        game->playSound(SND_JUMP);
        dashDelay = 400;
      }
    }

    if(dashDelay > 0)
    {
      wantedSpeed *= 4;
      vel.x = wantedSpeed;
    }

    vel.x = (vel.x * 0.95 + wantedSpeed * 0.05);

    if(abs(vel.x) < 0.00001)
      vel.x = 0;
  }

  bool move(Vector2f delta)
  {
    auto rect = getRect();
    rect.x += delta.x;
    rect.y += delta.y;

    const Vector2f vertices[] =
    {
      Vector2f(rect.x, rect.y + rect.height / 2.0),
      Vector2f(rect.x + rect.width, rect.y + rect.height / 2.0),
    };

    for(auto& v : vertices)
      if(game->isSolid(v))
        return false;

    if(game->isSolid(rect, rect))
      return false;

    pos += delta;
    return true;
  }

  virtual void tick() override
  {
    decrement(blinking);
    decrement(hurtDelay);
    decrement(dashDelay);

    time++;
    computeVelocity(control);

    // horizontal move
    move(Vector2f(vel.x, 0));

    // vertical move

    if(move(Vector2f(0, vel.y)))
    {
      ground = false;
    }
    else
    {
      if(vel.y < 0 && !ground)
      {
        if(tryActivate(debounceLanding, 150))
          game->playSound(SND_LAND);

        ground = true;
        dashDelay = 0;
      }

      vel.y = 0;
    }

    decrement(debounceFire);
    decrement(debounceLanding);
    decrement(climbDelay);
    decrement(shootDelay);

    if(upgrades & UPGRADE_SHOOT)
    {
      if(firebutton.toggle(control.fire) && tryActivate(debounceFire, 150))
      {
        auto b = make_unique<Bullet>();
        auto sign = (dir == LEFT ? -1 : 1);
        auto offsetV = vel.x ? Vector2f(0, 1) : Vector2f(0, 0.9);
        auto offsetH = vel.x ? Vector2f(0.8, 0) : Vector2f(0.7, 0);
        b->pos = pos + offsetV + offsetH * sign;
        b->vel = Vector2f(0.025, 0) * sign;
        game->spawn(b.release());
        game->playSound(SND_FIRE);
        shootDelay = 300;
      }
    }

    collisionGroup = blinking ? 0b1000 : 0b1001;
  }

  virtual void onDamage(int amount) override
  {
    if(life <= 0)
      return;

    if(!blinking)
    {
      life -= amount;

      if(life < 0)
      {
        die();
        return;
      }

      hurtDelay = HURT_DELAY;
      blinking = 2000;
      game->playSound(SND_HURT);
    }
  }

  bool facingWall() const
  {
    auto const front = dir == RIGHT ? 0.7 : -0.7;

    if(game->isSolid(pos + Vector2f(size.width / 2 + front, 0.3)))
      return true;

    if(game->isSolid(pos + Vector2f(size.width / 2 + front, 1.2)))
      return true;

    return false;
  }

  void die()
  {
    game->playSound(SND_DIE);
  }

  Int debounceFire;
  Int debounceLanding;
  ORIENTATION dir;
  Bool ground;
  Toggle jumpbutton, firebutton, dashbutton;
  Int time;
  Int climbDelay;
  Int hurtDelay;
  Int dashDelay;
  Int shootDelay;
  Int life;
  Bool doubleJumped;
  Control control;

  Int upgrades;
};

std::unique_ptr<Player> makeRockman()
{
  return make_unique<Rockman>();
}

