#include <algorithm>

#include "game/entities/wheel.h"
#include "game/entities/bonus.h"
#include "game/entities/explosion.h"

#include "tests/tests.h"

unittest("Entity: explosion")
{
  auto explosion = makeExplosion();

  assert(!explosion->dead);
  assertEquals(0, explosion->getActor().ratio);

  for(int i = 0; i < 10000; ++i)
    explosion->tick();

  assert(explosion->dead);
  assertEquals(100, int(explosion->getActor().ratio * 100));
}

#include "game/entities/player.h"

struct NullPlayer : Player
{
  virtual void think(Control const &)
  {
  }

  virtual float health()
  {
    return 0;
  }

  virtual void addUpgrade(int)
  {
  }

  virtual Actor getActor() const override
  {
    return Actor(pos, 0);
  }
};

struct NullGame : IGame
{
  virtual void playSound(SOUND)
  {
  }

  virtual void spawn(Entity*)
  {
  }

  virtual void postEvent(unique_ptr<Event> )
  {
  }

  virtual void subscribeForEvents(IEventSink*)
  {
  }

  virtual Vector2f getPlayerPosition()
  {
    return Vector2f(0, 0);
  }
};

struct NullPhysics : IPhysics
{
  // called by game
  void addBody(Body*)
  {
    assert(false);
  }

  void removeBody(Body*)
  {
    assert(false);
  }

  void clearBodies()
  {
    assert(false);
  }

  void checkForOverlaps()
  {
    assert(false);
  }

  void setEdifice(function<bool(Rect2f)> )
  {
    assert(false);
  }

  // called by entities
  bool moveBody(Body* body, Vector2f delta)
  {
    auto rect = body->getRect();
    rect.x += delta.x;
    rect.y += delta.y;

    if(isSolid(rect))
      return false;

    body->pos += delta;
    return true;
  }

  bool isSolid(Rect2f rect) const
  {
    return rect.y < 0;
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

    int upgrades = 0;
  };

  NullGame game;
  NullPhysics physics;
  MockPlayer player;

  auto ent = makeBonus(0, 4);
  ent->game = &game;
  ent->physics = &physics;

  assert(!ent->dead);
  assertEquals(0, player.upgrades);

  ent->onCollide(&player);

  assert(ent->dead);
  assertEquals(4, player.upgrades);
}

bool nearlyEquals(float expected, float actual)
{
  return abs(expected - actual) < 0.01;
}

unittest("Entity: animate")
{
  auto ent = makeBonus(0, 4);

  float minVal = 10.0f;
  float maxVal = -10.0f;

  for(int i = 0; i < 1000; ++i)
  {
    auto actor = ent->getActor();
    minVal = min(minVal, actor.ratio);
    maxVal = max(maxVal, actor.ratio);
    ent->tick();
  }

  assert(nearlyEquals(0, minVal));
  assert(nearlyEquals(1, maxVal));
}

#include "game/entities/rockman.h"

unittest("Entity: rockman falls")
{
  auto player = makeRockman();
  auto game = NullGame();
  auto physics = NullPhysics();
  player->game = &game;
  player->physics = &physics;
  player->pos.y = 10;
  player->tick();

  assertEquals(ACTION_FALL, player->getActor().action);
}

unittest("Entity: rockman stands on ground, then walks")
{
  auto player = makeRockman();
  auto game = NullGame();
  auto physics = NullPhysics();
  player->game = &game;
  player->physics = &physics;

  player->pos.y = 0;

  for(int i = 0; i < 10; ++i)
    player->tick();

  assertEquals(ACTION_STAND, player->getActor().action);

  {
    Control cmd {};
    cmd.right = true;
    player->think(cmd);
  }

  player->tick();

  assertEquals(ACTION_WALK, player->getActor().action);
  assert(player->getActor().scale.width > 0);

  {
    Control cmd {};
    cmd.left = true;
    player->think(cmd);
  }

  player->tick();

  assertEquals(ACTION_WALK, player->getActor().action);
  assert(player->getActor().scale.width < 0);
}

