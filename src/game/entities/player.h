#pragma once

#include <algorithm>

#include "engine/raii.h"
#include "engine/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"

auto const SHIP_SPEED = 0.01;
auto const BULLET_SPEED = 0.20;

class Player : public Entity
{
public:
  Player()
  {
    ground = false;
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_BASE);
    r.scale = Vector2f(0.5, 0.5);
    return r;
  }

  void think(Control const& c)
  {
    {
      float dx = 0;

      if(c.left)
        dx -= SHIP_SPEED;

      if(c.right)
        dx += SHIP_SPEED;

      vel.x = dx;
    }

    // gravity
    vel.y -= 0.00005;

    if(c.up && ground)
    {
      game->playSound(SND_CHIRP);
      vel.y = 0.02;
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
        game->playSound(SND_LAND);
        ground = true;
      }

      vel.y = 0;
    }

    cooldown = max(cooldown - 1, 0);

    if(c.fire && cooldown == 0)
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

    if(game->isSolid(nextPos))
      return false;

    if(game->isSolid(nextPos + Vector2f(0.5, 0)))
      return false;

    pos = nextPos;
    return true;
  }

  virtual void tick() override
  {
    lifetime = 0;
    dead = false;
  }

  virtual void onCollide(Entity*) override
  {
    blinking = 100;
  }

  Int cooldown;
  Vector2f vel;
  bool ground;
};

