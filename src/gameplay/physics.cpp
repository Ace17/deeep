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

  bool moveBody(Body* body, Vector delta)
  {
    auto frect = body->getBox();
    frect.pos += delta;

    auto const irect = frect;

    auto const blocked = isSolid(body, irect);

    if(blocked)
    {
      if(auto blocker = getSolidBodyInBox(irect, -1, body))
        collideBodies(*body, *blocker);
    }
    else
    {
      auto oldSolid = body->solid;
      body->solid = false; // make pusher non-solid, so stacked bodies can move down

      if(body->pusher)
        pushOthers(body, irect, delta);

      body->pos = frect.pos;
      body->solid = oldSolid;
      // assert(!getSolidBodyInBox(body->getBox(), -1, body));
    }

    // update floor
    if(!body->pusher)
    {
      auto feet = body->getBox();
      feet.size.height = 0.1;
      feet.pos.y = body->getBox().pos.y - feet.size.height;
      body->floor = getSolidBodyInBox(feet, -1, body);
    }

    return !blocked;
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
    if(getSolidBodyInBox(rect, except->collidesWith, except))
      return true;

    if(m_isSolid(rect))
      return true;

    return false;
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

  void setEdifice(function<bool(Box)> edifice)
  {
    m_isSolid = edifice;
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

      auto rect = body->getBox();

      if(overlaps(rect, myBox))
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
  function<bool(Box)> m_isSolid;
};
}

unique_ptr<IPhysics> createPhysics()
{
  return make_unique<Physics>();
}

