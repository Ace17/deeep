#pragma once

#include "game/entity.h"

inline
bool move(Entity* ent, Vector2f delta)
{
  auto rect = ent->getRect();
  rect.x += delta.x;
  rect.y += delta.y;

  const Vector2f vertices[] =
  {
    Vector2f(rect.x, rect.y + rect.height / 2.0),
    Vector2f(rect.x + rect.width, rect.y + rect.height / 2.0),
  };

  for(auto& v : vertices)
    if(ent->game->isPointSolid(v))
      return false;

  if(ent->game->isSolid(rect, rect))
    return false;

  ent->pos += delta;
  return true;
}

