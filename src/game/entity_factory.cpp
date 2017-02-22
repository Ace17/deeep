#include "entity_factory.h"
#include <map>
#include <functional>

using namespace std;

typedef function<unique_ptr<Entity>()> CreationFunc;
static map<string, CreationFunc> getRegistry();
static const map<string, CreationFunc> registry = getRegistry();

unique_ptr<Entity> createEntity(string name)
{
  auto i_func = registry.find(name);

  if(i_func == registry.end())
    throw runtime_error("unknown entity type: '" + name + "'");

  return (*i_func).second();
}

#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/entities/spider.h"
#include "game/entities/teleporter.h"
#include "game/entities/detector.h"
#include "game/entities/bonus.h"
#include "game/entities/player.h"

static map<string, CreationFunc> getRegistry()
{
  map<string, CreationFunc> r;

  r[ENTITY_UPGRADE_CLIMB] =
    [] ()
    {
      return makeBonus(4, UPGRADE_CLIMB);
    };

  r[ENTITY_UPGRADE_SHOOT] =
    [] ()
    {
      return makeBonus(3, UPGRADE_SHOOT);
    };

  r[ENTITY_UPGRADE_DASH] =
    [] ()
    {
      return makeBonus(5, UPGRADE_DASH);
    };

  r[ENTITY_BONUS_LIFE] =
    [] ()
    {
      return makeBonus(0, 0);
    };

  r[ENTITY_ENEMY_WHEEL] =
    [] ()
    {
      return make_unique<Wheel>();
    };

  r[ENTITY_ENEMY_SPIDER] =
    [] ()
    {
      return make_unique<Spider>();
    };

  r[ENTITY_FRAGILE_DOOR] =
    [] ()
    {
      return make_unique<BreakableDoor>();
    };

  r[ENTITY_TELEPORTER] =
    [] ()
    {
      return make_unique<Teleporter>();
    };

  return r;
}

