#include "game/entity_factory.h"
#include "game/entities/switch.h"
#include "game/entities/detector.h"
#include "game/room.h"

int interpretTile(Vector2i ipos, Vector2i& start, IGame* game, int val, int& portalId)
{
  auto const pos = Vector2f(ipos.x, ipos.y);
  switch(val)
  {
  case ' ':
    return 0;
  case '!':
    start = ipos;
    return 0;
  case '?':
    {
      auto teleporter = createEntity(ENTITY_TELEPORTER);
      teleporter->pos = pos;
      game->spawn(teleporter.release());
      return 0;
    }
  case '@':
    {
      auto bonus = createEntity(ENTITY_BONUS_LIFE);
      bonus->pos = pos;
      game->spawn(bonus.release());
      return 0;
    }
  case '$':
    {
      auto bonus = createEntity(ENTITY_UPGRADE_SHOOT);
      bonus->pos = pos;
      game->spawn(bonus.release());
      return 0;
    }
  case '%':
    {
      auto bonus = createEntity(ENTITY_UPGRADE_CLIMB);
      bonus->pos = pos;
      game->spawn(bonus.release());
      return 0;
    }
  case '^':
    {
      auto bonus = createEntity(ENTITY_UPGRADE_DASH);
      bonus->pos = pos;
      game->spawn(bonus.release());
      return 0;
    }
  case '.':
    return 1;
  case 'X': return 5;
  case 'O': return 3;
  case 'Z': return 4;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
    {
      auto sw = new Switch(val - 'a');
      sw->pos = pos + Vector2f(0.3, 0.15);
      game->spawn(sw);
      return 0;
    }
  case 'A':
  case 'B':
  case 'C':
  case 'D':
    {
      auto sw = new Door(val - 'A', game);
      sw->pos = pos;
      game->spawn(sw);
      return 0;
    }
  case '#':
    {
      auto ent = createEntity(ENTITY_FRAGILE_DOOR);
      ent->pos = pos;
      game->spawn(ent.release());
      return 0;
    }
  case '*':
    {
      auto wh = createEntity(ENTITY_ENEMY_WHEEL);
      wh->pos = pos;
      game->spawn(wh.release());
      return 0;
    }
  case '&':
    {
      auto wh = createEntity(ENTITY_ENEMY_SPIDER);
      wh->pos = pos;
      game->spawn(wh.release());
      return 0;
    }
  case 'P':
    {
      auto portal = new Detector(portalId++);
      portal->pos = pos;
      portal->size = Size2f(0.1, 3);
      game->spawn(portal);
      return 0;
    }
  default:
    return 4;
  }
}

Room loadLevel(Matrix<char> const& input, IGame* game)
{
  Room r;
  r.tiles.resize(input.size);

  int portalId = 0;

  for(auto pos : rasterScan(input.size.width, input.size.height))
  {
    auto const x = pos.first;
    auto const y = pos.second;

    auto val = input.get(x, y);
    auto tile = interpretTile(Vector2i(x, y), r.start, game, val, portalId);

    r.tiles.set(x, y, tile);
  }

  return r;
}

