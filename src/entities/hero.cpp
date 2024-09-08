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
#include "gameplay/player.h"
#include "gameplay/sounds.h"
#include "gameplay/toggle.h"

#include "hero.h"

namespace
{
auto const WALK_SPEED = 0.075f;
auto const MAX_HORZ_SPEED = 0.2f;
auto const MAX_FALL_SPEED = 0.2f;
auto const CLIMB_DELAY = 10;
auto const HURT_DELAY = 50;
auto const JUMP_VEL = 0.15;
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
    collidesWith = CG_WALLS | CG_ENEMIES;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  void addActors(std::vector<Actor>& actors) const override
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

struct Bomb : Entity
{
  Bomb()
  {
    size = Size(0.4, 0.4);

    collidesWith = CG_WALLS | CG_ENEMIES;
  }

  void addActors(std::vector<Actor>& actors) const override
  {
    if(life == 0)
      return;

    auto r = Actor { pos, MDL_RECT };
    r.scale = size;
    r.zOrder = 2;

    actors.push_back(r);
  }

  void tick() override
  {
    if(decrement(life))
    {
      game->playSound(SND_EXPLODE);

      const auto center = getCenter();
      size = UnitSize * 1.5;
      pos = center - size * 0.5;

      Body::onCollision = [this] (Body* other) { onCollide(other); };
    }

    if(life == 0)
    {
      if(decrement(exploding))
        dead = true;
    }
  }

  void onCollide(Body* other)
  {
    if(auto damageable = dynamic_cast<Damageable*>(other))
      damageable->onDamage(10);
  }

  int life = 70;
  int exploding = 2;
};

static auto const NORMAL_SIZE = Size(0.7, 1.9);

struct Rockman : Entity, Damageable, Playerable
{
  Rockman(IEntityConfig*, Player* player_)
    : player(player_)
  {
    size = NORMAL_SIZE;
    collidesWith |= CG_LADDER;
    Body::onCollision = [this] (Body* other) { onCollide(other); };
  }

  Player* getPlayer() { return player; }

  void onCollide(Body* b)
  {
    if(dynamic_cast<Climbable*>(b))
    {
      ladderDelay = 10;
      ladderX = b->pos.x;
    }
  }

  void addActors(std::vector<Actor>& actors) const override
  {
    auto r = Actor { pos, MDL_ROCKMAN };
    r.scale = Size(3, 3);

    // re-center
    r.pos += Vector(-(r.scale.x - size.x) * 0.5, -0.0);

    if(ball)
    {
      r.action = ACTION_BALL;
      r.ratio = (time % 30) / 30.0f;
    }
    else if(sliding)
    {
      if(shootDelay == 0)
        r.action = ACTION_SLIDE;
      else
        r.action = ACTION_SLIDE_SHOOT;

      r.ratio = (time % 30) / 30.0f;
    }
    else if(hurtDelay || life < 0)
    {
      r.action = ACTION_HURT;
      r.ratio = 1.0f - hurtDelay / float(HURT_DELAY);
    }
    else if(ladder)
    {
      r.action = ACTION_LADDER;
      r.ratio = vel.y == 0 ? 0.3 : (time % 40) / 40.0f;
      r.pos += Vector(0.05, -0.5);
    }
    else if(!ground)
    {
      if(climbDelay)
      {
        r.action = ACTION_CLIMB;
        r.ratio = 1.0f - climbDelay / float(CLIMB_DELAY);
        r.scale.x *= -1;
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
          r.ratio = std::min(40 - dashDelay, 40) / 10.0f;
          r.action = ACTION_DASH;
        }
        else
        {
          r.ratio = (time % 50) / 50.0f;

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
          r.ratio = (time % 300) / 300.0f;
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
      r.scale.x *= -1;

    if(blinking)
      r.effect = Effect::Blinking;

    r.zOrder = 1;

    actors.push_back(r);
  }

  void think(Control const& c)
  {
    control = c;
  }

  float health()
  {
    return ::clamp(life / float(MAX_LIFE), 0.0f, 1.0f);
  }

  void addUpgrade(int upgrade)
  {
    upgrades |= upgrade;
    blinking = 200;
    life = MAX_LIFE;
    game->getVariable(-1)->set(upgrades);
  }

  void computeVelocity(Control c)
  {
    airMove(c);

    if(ground)
    {
      doubleJumped = false;

      if(!c.up)
        ladder = false;
    }

    if(vel.x > 0)
      dir = RIGHT;

    if(vel.x < 0)
      dir = LEFT;

    // gravity
    if(life > 0 && !ladder)
      vel.y -= 0.005;

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

          for(int i = 0; i < 8; ++i)
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
          dashDelay = 40;
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

    vel.x = ::clamp(vel.x, -MAX_HORZ_SPEED, MAX_HORZ_SPEED);
    vel.y = std::max(vel.y, -MAX_FALL_SPEED);
  }

  void airMove(Control c)
  {
    float wantedSpeed = 0;

    if(ladderDelay && (c.up || (c.down && !ground)) && !ball)
    {
      if(!ladder)
      {
        pos.x = ladderX + 0.1;
        vel.x = 0;
        ladder = true;
      }
    }

    if(ladder)
    {
      if(c.jump || c.left || c.right)
      {
        ladder = false;
      }
      else
      {
        if(c.up)
          vel.y = +WALK_SPEED * 0.5;
        else if(c.down)
          vel.y = -WALK_SPEED * 0.5;
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
        dashDelay = 40;
      }
    }

    if(dashDelay > 0)
    {
      wantedSpeed *= 4;
      vel.x = wantedSpeed;
    }

    for(int i = 0; i < 10; ++i)
      vel.x = (vel.x * 0.95 + wantedSpeed * 0.05);

    if(abs(vel.x) < 0.001)
      vel.x = 0;
  }

  void tick() override
  {
    decrement(blinking);
    decrement(hurtDelay);

    if(ground)
      decrement(dashDelay);

    if(hurtDelay || life <= 0)
    {
      control = Control {};
    }

    if(restartbutton.toggle(control.restart))
      life = 0;

    if(life && crushed)
    {
      life = -1;
      die();
    }

    // 'dying' animation
    if(life <= 0)
    {
      decrement(dieDelay);

      if(dieDelay < 100)
        game->setAmbientLight((dieDelay - 100) / 100.0);

      if(dieDelay == 0)
        respawn();
    }

    time++;
    computeVelocity(control);

    auto trace = slideMove(this, vel);

    if(!trace.vert)
      vel.y = 0;

    auto const wasOnGround = ground;

    // probe for solid ground
    {
      Box box;
      box.pos.x = pos.x;
      box.pos.y = pos.y - 0.1;
      box.size.x = size.x;
      box.size.y = 0.1;
      ground = physics->isSolid(box, this);
    }

    if(ground && !wasOnGround)
    {
      if(vel.y < 0)
      {
        game->playSound(SND_LAND);
        dashDelay = 0;
      }
    }

    decrement(debounceFire);
    decrement(climbDelay);
    decrement(shootDelay);
    decrement(ladderDelay);

    handleShooting();

    handleBall();
    collisionGroup = CG_PLAYER;

    if(!blinking)
      collisionGroup |= CG_SOLIDPLAYER;
  }

  void onDamage(int amount) override
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
      blinking = 200;
      game->playSound(SND_HURT);
    }
  }

  bool facingWall() const
  {
    auto const front = dir == RIGHT ? 0.7 : -0.7;

    Box box;
    box.pos.x = pos.x + size.x / 2 + front;
    box.pos.y = pos.y + 0.3;
    box.size.x = 0.01;
    box.size.y = 0.9;

    if(physics->isSolid(box, this))
      return true;

    return false;
  }

  void enter() override
  {
    game->setAmbientLight(0);
    upgrades = game->getVariable(-1)->get();
    ladderDelay = 0;
  }

  void die()
  {
    if(dieDelay)
      return;

    game->playSound(SND_DIE);
    ball = false;
    size = NORMAL_SIZE;
    dieDelay = 150;
  }

  void respawn()
  {
    game->respawn();
    blinking = 20;
    vel = NullVector;
    life = MAX_LIFE;
    crushed = false;
  }

  void handleShooting()
  {
    if(ball)
    {
      if(upgrades & UPGRADE_BOMB)
      {
        if(firebutton.toggle(control.fire) && tryActivate(debounceFire, 15))
        {
          auto b = std::make_unique<Bomb>();
          b->pos = getCenter() - b->size * 0.5;
          game->spawn(b.release());
          game->playSound(SND_FIRE);
        }
      }
    }
    else
    {
      if(upgrades & UPGRADE_SHOOT)
      {
        if(firebutton.toggle(control.fire) && tryActivate(debounceFire, 15))
        {
          auto b = std::make_unique<Bullet>();
          auto sign = (dir == LEFT ? -1 : 1);
          auto offsetV = Vector(0, 1);
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
          shootDelay = 30;
        }
      }
    }
  }

  void handleBall()
  {
    if(!ladder && control.down && !ball && (upgrades & UPGRADE_BALL))
    {
      ball = true;
      size = Size(NORMAL_SIZE.x, 0.9);
    }

    if(control.up && ball)
    {
      Box box;
      box.size = NORMAL_SIZE;
      box.pos = pos;

      if(!physics->isSolid(box, this))
      {
        ball = false;
        size = NORMAL_SIZE;
      }
    }
  }

  int debounceFire = 0;
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
  Player* const player;
};

struct HeroPlayer : Player
{
  HeroPlayer(IGame* game_) : m_entity(nullptr, this), game(game_)
  {}

  void think(Control const& c)
  {
    m_entity.think(c);
  }

  float health()
  {
    return m_entity.health();
  }

  Vector position() override
  {
    return m_entity.pos;
  }

  void setPosition(Vector pos) override
  {
    m_entity.pos = pos;
  }

  void addUpgrade(int upgrade)
  {
    m_entity.addUpgrade(upgrade);
  }

  void enterLevel() override { game->spawn(&m_entity); };
  void leaveLevel() override { game->detach(&m_entity); };

  Rockman m_entity;
  IGame* const game;
};
}

Player* createHeroPlayer(IGame* game)
{
  return new HeroPlayer(game);
}

