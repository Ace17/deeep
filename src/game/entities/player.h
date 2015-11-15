#pragma once

#include <algorithm>

#include "engine/util.h"
#include "engine/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"
#include "game/toggle.h"

auto const SHIP_SPEED = 0.0075;
auto const BULLET_SPEED = 0.20;

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

class Player : public Entity
{
public:
  Player()
  {
    ground = false;
    size = Dimension2f(0.5, 0.5);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos + Vector2f(0, -0.1), MDL_ROCKMAN);

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

  void think(Control const& c)
  {
    ++time;
    {
      float dx = 0;

      if(c.left)
        dx -= SHIP_SPEED;

      if(c.right)
        dx += SHIP_SPEED;

      vel.x = dx;

      if(vel.x > 0)
        orientedRight = true;

      if(vel.x < 0)
        orientedRight = false;
    }

    // gravity
    vel.y -= 0.00005;

    if(jumpbutton.toggle(c.up) && ground)
    {
      game->playSound(SND_JUMP);
      vel.y = 0.015;
    }

    if(vel.y > 0 && !c.up)
      vel.y = 0;

    vel.x = min(vel.x, 0.02f);
    vel.x = max(vel.x, -0.02f);

    // limit falling speed
    vel.y = max(vel.y, -0.02f);

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

