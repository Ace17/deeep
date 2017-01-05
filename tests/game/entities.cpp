#include "game/entities/wheel.h"
#include "game/entities/explosion.h"

#include "tests/tests.h"

unittest("Entity: explosion")
{
  auto explosion = makeExplosion();

  assert(!explosion->dead);
  assert(explosion->getActor().ratio == 0);

  for(int i = 0; i < 10000; ++i)
    explosion->tick();

  assert(explosion->dead);
  assert(int(explosion->getActor().ratio * 100) == 100);
}

