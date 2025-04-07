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
SpriteActor getActor(T& ent)
{
  std::vector<SpriteActor> actors;
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
  virtual void think(Control const &) {}
  virtual float health() { return 0; }
  virtual void addUpgrade(int) {}
};

struct NullVariable : IVariable
{
  int get() { return 0; }
  void set(int) {}
  std::unique_ptr<Handle> observe(Observer &&) { return nullptr; }
};

static NullVariable nullVariable;

struct NullGame : IGame
{
  virtual void playSound(SOUND) {}
  virtual void stopMusic() {}
  virtual void spawn(Entity* e) { entity = e; e->physics = physicsProbe; }
  virtual void detach(Entity*) {}
  virtual IVariable* getVariable(int) { return &nullVariable; }
  virtual void postEvent(std::unique_ptr<Event>) {}
  virtual Vec2f getPlayerPosition() { return {}; }
  virtual void textBox(char const*) {}
  virtual void setAmbientLight(float) {}
  virtual void respawn() {}

  IPhysicsProbe* physicsProbe = nullptr;
  Entity* entity = nullptr;
};

struct NullPhysicsProbe : IPhysicsProbe
{
  // called by entities
  float moveBody(Body* body, Vec2f delta)
  {
    auto rect = body->getBox();
    rect.pos.x += delta.x;
    rect.pos.y += delta.y;

    if(isSolid(rect, body))
      return 0;

    body->pos += delta;
    return 1;
  }

  bool isSolid(Box rect, const Body* /*except*/) const
  {
    return rect.pos.y < 0;
  }

  Body* getBodiesInBox(Box, int, bool, const Body*) const
  {
    return nullptr;
  }
};

unittest("Entity: pickup bonus")
{
  struct MockPlayer : NullPlayer
  {
    virtual void addUpgrade(int upgrade)
    {
      upgrades |= upgrade;
    }

    Vector position() override { return {}; }
    void setPosition(Vector) override {};

    int upgrades = 0;
  };

  struct MockEntity : Entity, Playerable
  {
    Player* getPlayer() override
    {
      return &player;
    }

    void addActors(std::vector<SpriteActor> &) const override {}
    MockPlayer player;
  };

  NullGame game;
  NullPhysicsProbe physics;
  MockEntity playerEntity;

  auto ent = makeBonus(0, 4, "cool text");
  ent->game = &game;
  ent->physics = &physics;

  assert(!ent->dead);
  assertEquals(0, playerEntity.player.upgrades);

  ent->onCollision(&playerEntity);

  assert(ent->dead);
  assertEquals(4, playerEntity.player.upgrades);
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
    minVal = std::min(minVal, actor.ratio);
    maxVal = std::max(maxVal, actor.ratio);
    ent->tick();
  }

  assert(nearlyEquals(0, minVal));
  assert(nearlyEquals(1, maxVal));
}

#include "entities/hero.h"

unittest("Entity: hero falls")
{
  auto physics = NullPhysicsProbe();
  auto game = NullGame();
  game.physicsProbe = &physics;
  std::unique_ptr<Player> player(createHeroPlayer(&game));
  player->enterLevel();
  player->setPosition({ 0, 10 });
  // player->tick();

  assertEquals((int)ACTION_FALL, (int)getActor(game.entity).action);
}

#if 0
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
  assert(getActor(player).scale.x > 0);

  {
    Control cmd {};
    cmd.left = true;
    player->think(cmd);
  }

  player->tick();

  assertEquals((int)ACTION_WALK, (int)getActor(player).action);
  assert(getActor(player).scale.x < 0);
}
#endif

