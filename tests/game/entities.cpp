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

  virtual void addUpgrade(Int)
  {
  }

  virtual int getUpgrades()
  {
    return 0;
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

  virtual bool isSolid(Vector2f pos)
  {
    return pos.y < 0;
  }

  virtual void trigger(int)
  {
  }

  virtual void listen(int, ITriggerable*)
  {
  }
};

unittest("Entity: pickup bonus")
{
  struct MockPlayer : NullPlayer
  {
    virtual void addUpgrade(Int upgrade)
    {
      upgrades |= upgrade;
    }

    int upgrades = 0;
  };

  NullGame game;
  MockPlayer player;

  auto ent = makeBonus(0, 4);
  ent->game = &game;

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
  player->game = &game;
  player->tick();

  assertEquals(ACTION_FALL, player->getActor().action);
}

