#include "entity_factory.h"
#include <map>
#include <functional>

using namespace std;

typedef vector<string> const EntityArgs;

typedef function<unique_ptr<Entity>(EntityArgs & args)> CreationFunc;
static map<string, CreationFunc> getRegistry();
static const map<string, CreationFunc> registry = getRegistry();

unique_ptr<Entity> createEntity(string formula)
{
  EntityArgs args;
  auto name = formula;

  auto i_func = registry.find(name);

  if(i_func == registry.end())
    throw runtime_error("unknown entity type: '" + name + "'");

  return (*i_func).second(args);
}

#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/entities/spider.h"
#include "game/entities/hopper.h"
#include "game/entities/teleporter.h"
#include "game/entities/detector.h"
#include "game/entities/bonus.h"
#include "game/entities/player.h"

static map<string, CreationFunc> getRegistry()
{
  map<string, CreationFunc> r;

  r[ENTITY_UPGRADE_CLIMB] =
    [] (EntityArgs &)
    {
      return makeBonus(4, UPGRADE_CLIMB);
    };

  r[ENTITY_UPGRADE_SHOOT] =
    [] (EntityArgs &)
    {
      return makeBonus(3, UPGRADE_SHOOT);
    };

  r[ENTITY_UPGRADE_DASH] =
    [] (EntityArgs &)
    {
      return makeBonus(5, UPGRADE_DASH);
    };

  r[ENTITY_UPGRADE_DJUMP] =
    [] (EntityArgs &)
    {
      return makeBonus(6, UPGRADE_DJUMP);
    };

  r[ENTITY_BONUS_LIFE] =
    [] (EntityArgs &)
    {
      return makeBonus(0, 0);
    };

  r[ENTITY_ENEMY_WHEEL] =
    [] (EntityArgs &)
    {
      return make_unique<Wheel>();
    };

  r[ENTITY_ENEMY_HOPPER] =
    [] (EntityArgs &)
    {
      return make_unique<Hopper>();
    };

  r[ENTITY_ENEMY_SPIDER] =
    [] (EntityArgs &)
    {
      return make_unique<Spider>();
    };

  r[ENTITY_FRAGILE_DOOR] =
    [] (EntityArgs &)
    {
      return make_unique<BreakableDoor>();
    };

  r[ENTITY_TELEPORTER] =
    [] (EntityArgs &)
    {
      return make_unique<Teleporter>();
    };

  return r;
}

