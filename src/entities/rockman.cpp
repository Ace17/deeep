// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include <algorithm>

#include "base/scene.h"
#include "base/util.h"

#include "collision_groups.h"
#include "entities/player.h"
#include "entities/move.h"
#include "entity.h"
#include "models.h"
#include "sounds.h"
#include "toggle.h"

#include "rockman.h"

auto const WALK_SPEED = 0.0075f;
auto const MAX_HORZ_SPEED = 0.02f;
auto const MAX_FALL_SPEED = 0.02f;
auto const CLIMB_DELAY = 100;
auto const HURT_DELAY = 500;
auto const JUMP_VEL = 0.015;
auto const MAX_LIFE = 31;

enum ORIENTATION
{
  LEFT,
  RIGHT,
};

struct Bullet : Entity
{
  Bullet()
  {
    size = Size(0.5, 0.4);
    collisionGroup = 0;
    collidesWith = CG_WALLS;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_BULLET };
    r.scale = size;
    r.action = 0;
    r.ratio = 0;

    actors.push_back(r);
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
      damageable->onDamage(10);

    dead = true;
  }

  int life = 1000;
  Vector vel;
};

static auto const NORMAL_SIZE = Size(0.7, 1.9);

struct Rockman : Player, Damageable
{
  Rockman()
  {
    size = NORMAL_SIZE;
    collidesWith |= CG_LADDER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void onCollide(Body* b)
  {
    if(dynamic_cast<Climbable*>(b))
    {
      ladderDelay = 10;
      ladderX = b->pos.x;
    }
  }

  virtual void addActors(vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_ROCKMAN };
    r.scale = Size(3, 3);

    // re-center
    r.pos += Vector(-(r.scale.width - size.width) * 0.5, -0.1);

    if(ball)
    {
      r.action = ACTION_BALL;
      r.ratio = (time % 300) / 300.0f;
    }
    else if(sliding)
    {
      if(shootDelay == 0)
        r.action = ACTION_SLIDE;
      else
        r.action = ACTION_SLIDE_SHOOT;

      r.ratio = (time % 300) / 300.0f;
    }
    else if(hurtDelay || life < 0)
    {
      r.action = ACTION_HURT;
      r.ratio = 1.0f - hurtDelay / float(HURT_DELAY);
    }
    else if(ladder)
    {
      r.action = ACTION_LADDER;
      r.ratio = vel.y == 0 ? 0.3 : (time % 200) / 200.0f;
      r.pos += Vector(0.05, -0.5);
    }
    else if(!ground)
    {
      if(climbDelay)
      {
        r.action = ACTION_CLIMB;
        r.ratio = 1.0f - climbDelay / float(CLIMB_DELAY);
        r.scale.width *= -1;
      }
      else
      {
        r.pos.y -= 0.3;
        r.action = shootDelay ? ACTION_FALL_SHOOT : ACTION_FALL;
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
          r.ratio = (time % 3000) / 3000.0f;
          r.action = ACTION_STAND;
        }
        else
        {
          r.ratio = 0;
          r.action = ACTION_STAND_SHOOT;
        }
      }
    }

    if(dir == LEFT)
      r.scale.width *= -1;

    if(blinking)
      r.effect = Effect::Blinking;

    r.zOrder = 1;

    actors.push_back(r);
  }

  void think(Control const& c) override
  {
    control = c;
  }

  float health() override
  {
    return clamp(life / float(MAX_LIFE), 0.0f, 1.0f);
  }

  virtual void addUpgrade(int upgrade) override
  {
    upgrades |= upgrade;
    blinking = 2000;
    life = MAX_LIFE;
    game->getVariable(-1)->set(upgrades);
  }

  void computeVelocity(Control c)
  {
    airMove(c);

    if(ground)
      doubleJumped = false;

    if(vel.x > 0)
      dir = RIGHT;

    if(vel.x < 0)
      dir = LEFT;

    // gravity
    if(life > 0 && !ladder)
      vel.y -= 0.00005;

    sliding = false;

    if(upgrades & UPGRADE_SLIDE)
    {
      if(!ball && !ground)
      {
        if(vel.y < 0 && facingWall() && (c.left || c.right))
        {
          // don't allow double-jumping from sliding state,
          // unless we have the climb upgrade
          doubleJumped = !(upgrades & UPGRADE_CLIMB);
          vel.y *= 0.97;
          sliding = true;
          dashDelay = 0;
        }
      }
    }

    if(jumpbutton.toggle(c.jump))
    {
      if(ground)
      {
        game->playSound(SND_JUMP);
        vel.y = JUMP_VEL;
        doubleJumped = false;
      }
      else if(facingWall() && (upgrades & UPGRADE_CLIMB))
      {
        game->playSound(SND_JUMP);
        // wall climbing
        vel.x = dir == RIGHT ? -0.04 : 0.04;

        if(c.dash)
          dashDelay = 400;
        else
          dashDelay = 0;

        vel.y = JUMP_VEL;
        climbDelay = CLIMB_DELAY;
        doubleJumped = false;
      }
      else if((upgrades & UPGRADE_DJUMP) && !doubleJumped)
      {
        game->playSound(SND_JUMP);
        vel.y = JUMP_VEL;
        doubleJumped = true;
      }
    }

    if(!ladder)
    {
      // stop jump if the player release the button early
      if(vel.y > 0 && !c.jump)
        vel.y = 0;
    }

    vel.x = clamp(vel.x, -MAX_HORZ_SPEED, MAX_HORZ_SPEED);
    vel.y = max(vel.y, -MAX_FALL_SPEED);
  }

  void airMove(Control c)
  {
    float wantedSpeed = 0;

    if(ladderDelay && (c.up || c.down))
      ladder = true;

    if(ladder)
    {
      pos.x = ladderX + 0.1;

      if(c.jump || c.left || c.right)
      {
        ladder = false;
        ground = false;
      }
      else
      {
        if(c.up)
          vel.y = +WALK_SPEED;
        else if(c.down)
          vel.y = -WALK_SPEED;
        else
          vel.y = 0;
      }
    }

    if(!climbDelay && !ladder)
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

  virtual void tick() override
  {
    for(int i = 0; i < 10; ++i)
      subTick();
  }

  void subTick()
  {
    decrement(blinking);
    decrement(hurtDelay);

    if(ground)
      decrement(dashDelay);

    if(hurtDelay || life <= 0)
      control = Control {};

    if(restartbutton.toggle(control.restart))
      life = 0;

    // 'dying' animation
    if(life <= 0)
    {
      decrement(dieDelay);

      if(dieDelay < 1000)
        game->setAmbientLight((dieDelay - 1000) * 0.001);

      if(dieDelay == 0)
        respawn();
    }

    time++;
    computeVelocity(control);

    auto trace = slideMove(this, vel);

    if(trace.vert && !Body::floor)
      ground = false;

    if(!trace.vert)
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
    decrement(ladderDelay);

    handleShooting();

    handleBall();
    collisionGroup = CG_PLAYER;

    if(!blinking)
      collisionGroup |= CG_SOLIDPLAYER;
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

    Box box;
    box.pos.x = pos.x + size.width / 2 + front;
    box.pos.y = pos.y + 0.3;
    box.size.width = 0.01;
    box.size.height = 0.9;

    if(physics->isSolid(this, roundBox(box)))
      return true;

    return false;
  }

  void enter() override
  {
    game->setAmbientLight(0);
    upgrades = game->getVariable(-1)->get();
  }

  void die()
  {
    game->playSound(SND_DIE);
    ball = false;
    size = NORMAL_SIZE;
    dieDelay = 1500;
  }

  void respawn()
  {
    game->respawn();
    blinking = 2000;
    vel = NullVector;
    life = MAX_LIFE;
  }

  void handleShooting()
  {
    if(upgrades & UPGRADE_SHOOT && !ball)
    {
      if(firebutton.toggle(control.fire) && tryActivate(debounceFire, 150))
      {
        auto b = make_unique<Bullet>();
        auto sign = (dir == LEFT ? -1 : 1);
        auto offsetV = vel.x ? Vector(0, 1) : Vector(0, 0.9);
        auto offsetH = vel.x ? Vector(0.8, 0) : Vector(0.7, 0);

        if(sliding)
        {
          sign = -sign;
        }
        else if(!ground)
          offsetV.y += 0.25;

        b->pos = pos + offsetV + offsetH * sign;
        b->vel = Vector(0.25, 0) * sign;
        game->spawn(b.release());
        game->playSound(SND_FIRE);
        shootDelay = 300;
      }
    }
  }

  void handleBall()
  {
    if(!ladder && control.down && !ball && (upgrades & UPGRADE_BALL))
    {
      ball = true;
      size = Size(NORMAL_SIZE.width, 0.9);
    }

    if(control.up && ball)
    {
      Box box;
      box.size = NORMAL_SIZE;
      box.pos = pos;

      if(!physics->isSolid(this, roundBox(box)))
      {
        ball = false;
        size = NORMAL_SIZE;
      }
    }
  }

  int debounceFire = 0;
  int debounceLanding = 0;
  ORIENTATION dir = RIGHT;
  bool ground = false;
  Toggle jumpbutton, firebutton, dashbutton, restartbutton;
  int time = 0;
  int climbDelay = 0;
  int hurtDelay = 0;
  int dashDelay = 0;
  int dieDelay = 0;
  int shootDelay = 0;
  int ladderDelay = 0;
  float ladderX;
  int life = MAX_LIFE;
  bool doubleJumped = false;
  bool ball = false;
  bool sliding = false;
  bool ladder = false;
  Control control {};
  Vector vel;

  int upgrades = 0;
};

std::unique_ptr<Player> makeRockman()
{
  return make_unique<Rockman>();
}

