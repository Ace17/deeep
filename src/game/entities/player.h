#pragma once

#include "engine/raii.h"
#include "engine/scene.h"
#include "game/entity.h"

auto const SHIP_SPEED = 0.10;
auto const BULLET_SPEED = 0.20;

class Player : public Entity
{
public:
  virtual Actor getActor() const override
  {
    return Actor(pos, MODEL_BASE);
  }

  void think(Control const& c)
  {
    if(c.left)
      pos.x -= SHIP_SPEED;

    if(c.right)
      pos.x += SHIP_SPEED;

    if(c.down)
      pos.y -= SHIP_SPEED;

    if(c.up)
      pos.y += SHIP_SPEED;

    pos.x = max(pos.x, -45.0f);
    pos.x = min(pos.x, +45.0f);
    pos.y = max(pos.y, -45.0f);
    pos.y = min(pos.y, +45.0f);

    cooldown = max(cooldown - 1, 0);

    if(c.fire && cooldown == 0)
    {
      /*
         cooldown = 100;
         auto b = unique(new Bullet);
         b->pos = pos + Vector2f(0, 9);
         b->vel = Vector2f(0.0, BULLET_SPEED);
         game->spawn(b.release());
         game->playSound(SOUND_FIRE);
       */
    }
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
};

