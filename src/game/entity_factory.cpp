#include "entity_factory.h"
#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/entities/spider.h"
#include "game/entities/teleporter.h"
#include "game/entities/detector.h"
#include "game/entities/bonus.h"
#include "game/entities/player.h"
#include "game/room.h"

using namespace std;

unique_ptr<Entity> createEntity(string name)
{
  if(name == ENTITY_UPGRADE_CLIMB)
  {
    return makeBonus(4, UPGRADE_CLIMB);
  }
  else if(name == ENTITY_UPGRADE_SHOOT)
  {
    return makeBonus(3, UPGRADE_SHOOT);
  }
  else if(name == ENTITY_UPGRADE_DASH)
  {
    return makeBonus(5, UPGRADE_DASH);
  }
  else if(name == ENTITY_BONUS_LIFE)
  {
    return makeBonus(0, 0);
  }
  else if(name == ENTITY_ENEMY_WHEEL)
  {
    return make_unique<Wheel>();
  }
  else if(name == ENTITY_ENEMY_SPIDER)
  {
    return make_unique<Spider>();
  }
  else if(name == ENTITY_FRAGILE_DOOR)
  {
    return make_unique<BreakableDoor>();
  }
  else if(name == ENTITY_TELEPORTER)
  {
    return make_unique<Teleporter>();
  }
  else
    throw runtime_error("unknown entity type: '" + name + "'");
}

