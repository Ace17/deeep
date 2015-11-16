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

#include "engine/scene.h"
#include "engine/util.h"

#include "game/entities/player.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/toggle.h"

auto const WALK_SPEED = 0.0075f;
auto const MAX_HORZ_SPEED = 0.02f;
auto const MAX_FALL_SPEED = 0.02f;

enum ACTION
{
  ACTION_VICTORY,
  ACTION_STAND,
  ACTION_STAND_SHOOT,
  ACTION_ENTRANCE,
  ACTION_WALK,
  ACTION_WALK_SHOOT,
  ACTION_DASH,
  ACTION_DASH_AIM,
  ACTION_JUMP,
  ACTION_FALL,
  ACTION_LADDER,
  ACTION_LADDER_END,
  ACTION_LADDER_SHOOT,
  ACTION_HADOKEN,
  ACTION_SLIDE,
  ACTION_SLIDE_SHOOT,
  ACTION_CLIMB,
  ACTION_HURT,
  ACTION_FULL,
};

class Rockman : public Player
{
public:
  virtual Actor getActor() const override
  {
    auto actorPos = pos + Vector2f(size.width / 2 - 0.75, -0.1);
    auto r = Actor(actorPos, MDL_ROCKMAN);
    r.scale = Vector2f(1.25, 1.25);

    if(!ground)
    {
      r.action = ACTION_FALL;
      r.ratio = vel.y > 0 ? 0 : 1;
    }
    else if(vel.x != 0)
    {
      r.ratio = (time % 500) / 500.0f;
      r.action = ACTION_WALK;
    }
    else
    {
      r.ratio = (time % 1000) / 1000.0f;
      r.action = ACTION_STAND;
    }

    if(!orientedRight)
      r.scale.x *= -1;

    return r;
  }

  void think(Control const& c) override
  {
    ++time;
    computeVelocity(c);

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
        if(landingCooldown == 0)
        {
          game->playSound(SND_LAND);
          landingCooldown = 150;
        }

        ground = true;
      }

      vel.y = 0;
    }

    cooldown = max(cooldown - 1, 0);
    landingCooldown = max(landingCooldown - 1, 0);

    if(firebutton.toggle(c.fire) && cooldown == 0)
    {
      cooldown = 150;
      /*
         auto b = unique(new Bullet);
         b->pos = pos + Vector2f(0, 9);
         b->vel = Vector2f(0.0, BULLET_SPEED);
         game->spawn(b.release());
       */
      game->playSound(SND_BEEP);
    }
  }

  void computeVelocity(Control const& c)
  {
    {
      float wantedSpeed = 0;

      if(c.left)
        wantedSpeed -= WALK_SPEED;

      if(c.right)
        wantedSpeed += WALK_SPEED;

      vel.x = wantedSpeed;

      if(vel.x > 0)
        orientedRight = true;

      if(vel.x < 0)
        orientedRight = false;
    }

    // gravity
    vel.y -= 0.00005;

    if(jumpbutton.toggle(c.jump) && ground)
    {
      game->playSound(SND_JUMP);
      vel.y = 0.015;
    }

    if(vel.y > 0 && !c.jump)
      vel.y = 0;

    vel.x = clamp(vel.x, -MAX_HORZ_SPEED, MAX_HORZ_SPEED);
    vel.y = max(vel.y, -MAX_FALL_SPEED);
  }

  bool move(Vector2f delta)
  {
    auto nextPos = pos + delta;

    if(game->isSolid(nextPos + Vector2f(0.10, 0)))
      return false;

    if(game->isSolid(nextPos + Vector2f(0.60, 0)))
      return false;

    if(game->isSolid(nextPos + Vector2f(0.10, 0.80)))
      return false;

    if(game->isSolid(nextPos + Vector2f(0.60, 0.80)))
      return false;

    pos = nextPos;
    return true;
  }

  virtual void tick() override
  {
    dead = false;
  }

  virtual void onCollide(Entity*) override
  {
    blinking = 100;
  }

  Int cooldown;
  Int landingCooldown;
  Vector2f vel;
  Bool orientedRight;
  Bool ground;
  Toggle jumpbutton, firebutton;
  int time;
};

Player* createRockman()
{
  return new Rockman;
}

