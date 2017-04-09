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
#include "game/entities/detector.h"
#include "game/entities/bonus.h"
#include "game/entities/player.h"
#include "game/entities/spikes.h"
#include "game/entities/blocks.h"
#include "game/entities/moving_platform.h"

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

  r[ENTITY_UPGRADE_BALL] =
    [] (EntityArgs &)
    {
      return makeBonus(7, UPGRADE_BALL);
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

  r[ENTITY_SPIKES] =
    [] (EntityArgs &)
    {
      return make_unique<Spikes>();
    };

  r[ENTITY_FRAGILE_DOOR] =
    [] (EntityArgs &)
    {
      return makeBreakableDoor();
    };

  r["fragile_block"] =
    [] (EntityArgs &)
    {
      return make_unique<FragileBlock>();
    };

  r["crumble_block"] =
    [] (EntityArgs &)
    {
      return make_unique<CrumbleBlock>();
    };

  r["door(0)"] =
    [] (EntityArgs &)
    {
      return makeDoor(0);
    };

  r["switch(0)"] =
    [] (EntityArgs &)
    {
      return makeSwitch(0);
    };

  r["moving_platform(0)"] =
    [] (EntityArgs &)
    {
      return make_unique<MovingPlatform>(0);
    };

  r["moving_platform(1)"] =
    [] (EntityArgs &)
    {
      return make_unique<MovingPlatform>(1);
    };

  return r;
}

