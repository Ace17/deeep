/*
 * Copyright (C) 2021 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cmath>

#include "entities/bonus.h"
#include "entities/explosion.h"

#include "tests.h"

template<typename T>
Actor getActor(T& ent)
{
  vector<Actor> actors;
  actors.clear();
  ent->addActors(actors);
  return actors[0];
}

unittest("Entity: explosion")
{
  auto explosion = makeExplosion();

  assert(!explosion->dead);
  assertEquals(0, getActor(explosion).ratio);

  for(int i = 0; i < 10000; ++i)
    explosion->tick();

  assert(explosion->dead);
  assertEquals(100, int(getActor(explosion).ratio * 100));
}

#include "gameplay/player.h"

struct NullPlayer : Player
{
  virtual void think(Control const&) {}
  virtual float health() { return 0; }
  virtual void addUpgrade(int) {}
  virtual void addActors(vector<Actor>&) const {}
};

struct NullVariable : IVariable
{
  int get() { return 0; }
  void set(int) {}
  unique_ptr<Handle> observe(Observer&&) { return nullptr; }
};

static NullVariable nullVariable;

struct NullGame : IGame
{
  virtual void playSound(SOUND) {}
  virtual void stopMusic() {}
  virtual void spawn(Entity*) {}
  virtual IVariable* getVariable(int) { return &nullVariable; }
  virtual void postEvent(unique_ptr<Event>) {}
  virtual Vec2f getPlayerPosition() { return Vec2f(0, 0); }
  virtual void textBox(char const*) {}
  virtual void setAmbientLight(float) {}
  virtual void respawn() {}
};

struct NullPhysicsProbe : IPhysicsProbe
{
  // called by entities
  float moveBody(Body* body, Vec2f delta)
  {
    auto rect = body->getBox();
    rect.pos.x += delta.x;
    rect.pos.y += delta.y;

    if(isSolid(body, rect))
      return 0;

    body->pos += delta;
    return 1;
  }

  bool isSolid(const Body* /*body*/, Box rect) const
  {
    return rect.pos.y < 0;
  }

  Body* getBodiesInBox(Box, int, bool, const Body*) const
  {
    return nullptr;
  }
};

float g_AmbientLight = 0;

unittest("Entity: pickup bonus")
{
  struct MockPlayer : NullPlayer
  {
    virtual void addUpgrade(int upgrade)
    {
      upgrades |= upgrade;
    }

    int upgrades = 0;
  };

  NullGame game;
  NullPhysicsProbe physics;
  MockPlayer player;

  auto ent = makeBonus(0, 4, "cool text");
  ent->game = &game;
  ent->physics = &physics;

  assert(!ent->dead);
  assertEquals(0, player.upgrades);

  ent->onCollision(&player);

  assert(ent->dead);
  assertEquals(4, player.upgrades);
}

bool nearlyEquals(float expected, float actual)
{
  return abs(expected - actual) < 0.01;
}

unittest("Entity: animate")
{
  auto ent = makeBonus(0, 4, "hello");

  float minVal = 10.0f;
  float maxVal = -10.0f;

  for(int i = 0; i < 1000; ++i)
  {
    auto actor = getActor(ent);
    minVal = min(minVal, actor.ratio);
    maxVal = max(maxVal, actor.ratio);
    ent->tick();
  }

  assert(nearlyEquals(0, minVal));
  assert(nearlyEquals(1, maxVal));
}

#include "entities/hero.h"

unittest("Entity: hero falls")
{
  auto player = makeRockman();
  auto game = NullGame();
  auto physics = NullPhysicsProbe();
  player->game = &game;
  player->physics = &physics;
  player->pos.y = 10;
  player->tick();

  assertEquals((int)ACTION_FALL, (int)getActor(player).action);
}

unittest("Entity: hero stands on ground, then walks")
{
  auto player = makeRockman();
  auto game = NullGame();
  auto physics = NullPhysicsProbe();
  player->game = &game;
  player->physics = &physics;

  player->pos.y = 0;

  for(int i = 0; i < 10; ++i)
    player->tick();

  assertEquals((int)ACTION_STAND, (int)getActor(player).action);

  {
    Control cmd {};
    cmd.right = true;
    player->think(cmd);
  }

  player->tick();

  assertEquals((int)ACTION_WALK, (int)getActor(player).action);
  assert(getActor(player).scale.width > 0);

  {
    Control cmd {};
    cmd.left = true;
    player->think(cmd);
  }

  player->tick();

  assertEquals((int)ACTION_WALK, (int)getActor(player).action);
  assert(getActor(player).scale.width < 0);
}

