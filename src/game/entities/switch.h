#pragma once

#include <algorithm>
#include <cmath>

#include "engine/raii.h"
#include "engine/scene.h"
#include "game/entity.h"
#include "game/models.h"
#include "game/sounds.h"

auto const SHIP_SPEED = 0.01;
auto const BULLET_SPEED = 0.20;

class Switch : public Entity
{
public:
  Switch(int id_) : id(id_)
  {
    size = Dimension2f(0.25, 0.25);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SWITCH);
    r.scale = Vector2f(0.35, 0.35);

    if(blinking)
      r.effect = EFFECT_BLINKING;

    r.frame = state ? 1 : 0;
    return r;
  }

  virtual void tick() override
  {
    blinking = max(0, blinking - 1);
  }

  virtual void onCollide(Entity*) override
  {
    if(blinking || state)
      return;

    blinking = 1200;
    state = !state;
    game->playSound(SND_SWITCH);
    game->trigger(id);
  }

  Bool state;
  const int id;
};

class Door : public Entity, public ITriggerable
{
public:
  Door(int id_, IGame* g) : id(id_)
  {
    game = g;
    game->listen(id, this);
    size = Dimension2f(1.2, 1.2);
  }

  virtual Actor getActor() const override
  {
    auto r = Actor(pos, MDL_SWITCH);
    r.scale = Vector2f(0.6, 0.6);
    r.frame = 2 + (state ? 1 : 0);
    return r;
  }

  virtual void tick() override
  {
  }

  virtual void onCollide(Entity* other) override
  {
    if(state)
      return;

    auto delta = other->pos - pos;

    if(abs(delta.x) < abs(delta.y))
      delta.x = 0;
    else
      delta.y = 0;

    other->pos += delta * 0.1;
  }

  virtual void trigger() override
  {
    state = !state;
  }

  Bool state;
  const int id;
};

