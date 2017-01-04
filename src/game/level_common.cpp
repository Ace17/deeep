#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/entities/teleporter.h"
#include "game/entities/bonus.h"

int interpretTile(Vector2i pos, Vector2i& start, IGame* game, int val)
{
  switch(val)
  {
  case ' ':
    return 0;
  case '!':
    start = pos;
    return 0;
  case '?':
    {
      auto teleporter = new Teleporter;
      teleporter->pos = Vector2f(pos.x, pos.y);
      game->spawn(teleporter);
      return 0;
    }
    return 0;
  case '@':
    {
      auto bonus = new Bonus(0, 0);
      bonus->pos = Vector2f(pos.x, pos.y);
      game->spawn(bonus);
      return 0;
    }
  case '$':
    {
      auto bonus = new Bonus(3, UPGRADE_SHOOT);
      bonus->pos = Vector2f(pos.x, pos.y);
      game->spawn(bonus);
      return 0;
    }
  case '%':
    {
      auto bonus = new Bonus(4, UPGRADE_CLIMB);
      bonus->pos = Vector2f(pos.x, pos.y);
      game->spawn(bonus);
      return 0;
    }
  case '^':
    {
      auto bonus = new Bonus(5, UPGRADE_DASH);
      bonus->pos = Vector2f(pos.x, pos.y);
      game->spawn(bonus);
      return 0;
    }
  default:
    return 4;
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
      sw->pos = Vector2f(pos.x + 0.3, pos.y + 0.15);
      game->spawn(sw);
      return 0;
    }
  case 'A':
  case 'B':
  case 'C':
  case 'D':
    {
      auto sw = new Door(val - 'A', game);
      sw->pos = Vector2f(pos.x, pos.y);
      game->spawn(sw);
      return 0;
    }
  case '#':
    {
      auto ent = new BreakableDoor;
      ent->pos = Vector2f(pos.x, pos.y);
      game->spawn(ent);
      return 0;
    }
  case '*':
    {
      auto wh = new Wheel;
      wh->pos = Vector2f(pos.x, pos.y);
      game->spawn(wh);
      return 0;
    }
  }
}

void loadLevel(Matrix<char> const& input, Matrix<int>& tiles, Vector2i& start, IGame* game)
{
  tiles.resize(input.size);

  for(int y = 0; y < input.size.height; ++y)
    for(int x = 0; x < input.size.width; ++x)
    {
      auto val = input.get(x, y);
      auto pos = Vector2i(x, y);
      auto tile = interpretTile(pos, start, game, val);

      tiles.set(x, y, tile);
    }
}

