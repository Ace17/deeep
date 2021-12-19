/*
 * Copyright (C) 2021 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cmath> // round

#include "base/util.h"
#include "body.h"
#include "physics.h"
#include <memory>
#include <vector>

using namespace std;

static const ShapeBox shapeBox;

Body::Body()
  : shape(&shapeBox)
{
}

namespace
{
struct Physics : IPhysics
{
  void addBody(Body* body)
  {
    m_bodies.push_back(body);
  }

  void removeBody(Body* body)
  {
    auto isItTheOne =
      [ = ] (Body* candidate) { return candidate == body; };
    unstableRemove(m_bodies, isItTheOne);
  }

  float moveBody(Body* body, Vector delta)
  {
    auto myBox = body->getBox();
    myBox.pos += delta;

    auto const blocker = getSolidBodyInBox(myBox, body->collidesWith, body);

    if(blocker)
    {
      collideBodies(*body, *blocker);
    }
    else
    {
      auto oldSolid = body->solid;
      body->solid = false; // make pusher non-solid, so stacked bodies can move down

      if(body->pusher)
        pushOthers(body, myBox, delta);

      body->pos = myBox.pos;
      body->solid = oldSolid;
    }

    // update floor
    if(!body->pusher)
    {
      auto feet = body->getBox();
      feet.size.height = 0.1;
      feet.pos.y = body->getBox().pos.y - feet.size.height;
      body->floor = getSolidBodyInBox(feet, body->collidesWith, body);
    }

    return blocker ? 0 : 1;
  }

  void pushOthers(Body* body, Box rect, Vector delta)
  {
    // move stacked bodies
    for(auto otherBody : m_bodies)
    {
      if(otherBody->floor == body)
        moveBody(otherBody, delta);
    }

    // push potential non-solid bodies
    for(auto other : m_bodies)
      if(other != body && overlaps(rect, other->getBox()))
        moveBody(other, delta);
  }

  bool isSolid(const Body* except, Box rect) const
  {
    return getSolidBodyInBox(rect, except->collidesWith, except);
  }

  void checkForOverlaps()
  {
    for(auto p : allPairs((int)m_bodies.size()))
    {
      auto& me = *m_bodies[p.first];
      auto& other = *m_bodies[p.second];

      auto rect = me.getBox();
      auto otherBox = other.getBox();

      if(overlaps(rect, otherBox))
        collideBodies(me, other);
    }
  }

  void collideBodies(Body& me, Body& other)
  {
    if(other.collidesWith & me.collisionGroup)
      other.onCollision(&me);

    if(me.collidesWith & other.collisionGroup)
      me.onCollision(&other);
  }

  Body* getBodiesInBox(Box myBox, int collisionGroup, bool onlySolid, const Body* except) const
  {
    for(auto& body : m_bodies)
    {
      if(onlySolid && !body->solid)
        continue;

      if(body == except)
        continue;

      if(!(body->collisionGroup & collisionGroup))
        continue;

      if(body->shape->probe(body, myBox))
        return body;
    }

    return nullptr;
  }

private:
  Body* getSolidBodyInBox(Box myBox, int collisionGroup, const Body* except) const
  {
    return getBodiesInBox(myBox, collisionGroup, true, except);
  }

  vector<Body*> m_bodies;
};
}

unique_ptr<IPhysics> createPhysics()
{
  return make_unique<Physics>();
}

bool ShapeBox::probe(Body* owner, Box otherBox) const
{
  return overlaps({ owner->pos, owner->size }, otherBox);
}

bool ShapeTilemap::probe(Body* /*owner*/, Box otherBox) const
{
  auto const x1 = otherBox.pos.x;
  auto const y1 = otherBox.pos.y;
  auto const x2 = otherBox.pos.x + otherBox.size.width;
  auto const y2 = otherBox.pos.y + otherBox.size.height;

  auto const col1 = int(floor(x1));
  auto const col2 = int(floor(x2));
  auto const row1 = int(floor(y1));
  auto const row2 = int(floor(y2));

  for(int row = row1; row <= row2; row++)
    for(int col = col1; col <= col2; col++)
      if(tiles->isInside(col, row) && tiles->get(col, row))
        return true;

  return false;
}

