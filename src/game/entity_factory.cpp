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
#include "game/entities/hopper.h"
#include "game/entities/sweeper.h"
#include "game/entities/detector.h"
#include "game/entities/bonus.h"
#include "game/entities/player.h"
#include "game/entities/spikes.h"
#include "game/entities/blocks.h"
#include "game/entities/moving_platform.h"
#include "game/entities/conveyor.h"

static map<string, CreationFunc> getRegistry()
{
  map<string, CreationFunc> r;

  r["upgrade_climb"] =
    [] (EntityArgs &)
    {
      return makeBonus(4, UPGRADE_CLIMB);
    };

  r["upgrade_shoot"] =
    [] (EntityArgs &)
    {
      return makeBonus(3, UPGRADE_SHOOT);
    };

  r["upgrade_dash"] =
    [] (EntityArgs &)
    {
      return makeBonus(5, UPGRADE_DASH);
    };

  r["upgrade_djump"] =
    [] (EntityArgs &)
    {
      return makeBonus(6, UPGRADE_DJUMP);
    };

  r["upgrade_ball"] =
    [] (EntityArgs &)
    {
      return makeBonus(7, UPGRADE_BALL);
    };

  r["upgrade_slide"] =
    [] (EntityArgs &)
    {
      return makeBonus(8, UPGRADE_SLIDE);
    };

  r["bonus_life"] =
    [] (EntityArgs &)
    {
      return makeBonus(0, 0);
    };

  r["wheel"] =
    [] (EntityArgs &)
    {
      return make_unique<Wheel>();
    };

  r["hopper"] =
    [] (EntityArgs &)
    {
      return make_unique<Hopper>();
    };

  r["sweeper"] =
    [] (EntityArgs &)
    {
      return make_unique<Sweeper>();
    };

  r["spider"] =
    [] (EntityArgs &)
    {
      extern unique_ptr<Entity> makeSpider();
      return makeSpider();
    };

  r["spikes"] =
    [] (EntityArgs &)
    {
      return make_unique<Spikes>();
    };

  r["fragile_door"] =
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

  r["elevator"] =
    [] (EntityArgs &)
    {
      return make_unique<Elevator>();
    };

  r["conveyor(0)"] =
    [] (EntityArgs &)
    {
      return make_unique<Conveyor>();
    };

  return r;
}

