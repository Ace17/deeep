// Copyright (C) 2023 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// physics engine: movement, collision detection and response.

#include <cmath> // floor

#include "base/util.h"
#include "body.h"
#include "misc/math.h"
#include "misc/stats.h"
#include "physics.h"
#include <memory>
#include <vector>

static const ShapeBox shapeBox;

Body::Body()
  : shape(&shapeBox)
{
}

namespace
{
Gauge ggOverlapChecks("Overlap checks");

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

    auto const rc = castBox(myBox, delta, body->collidesWith, body);

    if(rc.blocker)
      collideBodies(*body, *rc.blocker);

    if(rc.fraction > 0)
    {
      myBox.pos += rc.fraction * delta;

      auto oldSolid = body->solid;
      body->solid = false; // make pusher non-solid, so stacked bodies can move down

      if(body->pusher)
        pushOthers(body, myBox, rc.fraction * delta);

      body->pos = myBox.pos;
      body->solid = oldSolid;
    }

    // update floor
    if(!body->pusher)
    {
      auto feet = body->getBox();
      feet.size.y = 0.1;
      feet.pos.y = body->getBox().pos.y - feet.size.y;
      body->floor = getSolidBodyInBox(feet, body->collidesWith, body);
    }

    return rc.fraction;
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
        if(other->collisionGroup & body->collidesWith)
        {
          auto fraction = moveBody(other, delta);

          if(fraction < 1)
            other->crushed = true;
        }
  }

  bool isSolid(Box rect, const Body* except) const
  {
    return getSolidBodyInBox(rect, except->collidesWith, except);
  }

  void checkForOverlaps()
  {
    int checkCount = 0;

    for(auto p : allPairs((int)m_bodies.size()))
    {
      auto& me = *m_bodies[p.first];
      auto& other = *m_bodies[p.second];

      auto rect = me.getBox();
      auto otherBox = other.getBox();

      if(overlaps(rect, otherBox))
        collideBodies(me, other);

      ++checkCount;
    }

    ggOverlapChecks = checkCount;
  }

  void collideBodies(Body& me, Body& other)
  {
    if(other.collidesWith & me.collisionGroup)
      other.onCollision(&me);

    if(me.collidesWith & other.collisionGroup)
      me.onCollision(&other);
  }

  struct Raycast
  {
    float fraction = 1.0;
    Body* blocker = nullptr;
  };

  Raycast castBox(Box box, Vec2f delta, int collisionGroup, const Body* except) const
  {
    Raycast r;

    for(auto& body : m_bodies)
    {
      if(!body->solid)
        continue;

      if(body == except)
        continue;

      if(!(body->collisionGroup & collisionGroup))
        continue;

      AffineTransform transform;
      transform.scale = body->size;

      Box transformedBox;
      transformedBox.pos = box.pos - body->pos;
      transformedBox.size = box.size;

      const auto fraction = body->shape->raycast(transform, transformedBox, delta);

      if(fraction < r.fraction)
      {
        r.fraction = fraction;
        r.blocker = body;
      }
    }

    return r;
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

      AffineTransform transform;
      transform.scale = body->size;

      Box transformedBox = myBox;
      transformedBox.pos -= body->pos;

      if(body->shape->probe(transform, transformedBox))
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

Vec2f rotateLeft(Vec2f v) { return Vec2f(-v.y, v.x); }

// The obstacle is an AABB, whose position and halfSize are given as parameters.
// The return value represents the allowed move, as a fraction of the desired
// move (delta).
float raycastAgainstAABB(Vec2f pos, Vec2f delta, Vec2f obstaclePos, Vec2f obstacleHalfSize)
{
  const Vec2f axes[] = {
    { 1, 0 },
    { 0, 1 },
    rotateLeft(normalize(delta)),
  };

  float fraction = 0;

  for(auto axis : axes)
  {
    // make the move always increase the position along the axis
    if(dotProduct(axis, delta) < 0)
      axis = axis * -1;

    const float obstacleExtent =
      abs(obstacleHalfSize.x * axis.x) + abs(obstacleHalfSize.y * axis.y);

    // compute projections on the axis
    const float startPos = dotProduct(axis, pos);
    const float targetPos = dotProduct(axis, pos + delta);
    const float obstacleMin = dotProduct(axis, obstaclePos) - obstacleExtent;
    const float obstacleMax = dotProduct(axis, obstaclePos) + obstacleExtent;

    if(targetPos < obstacleMin)
      return 1; // all the axis-projected move is before the obstacle

    if(startPos >= obstacleMax)
      return 1; // all the axis-projected move is after the obstacle

    // don't update 'fraction' if the move is parallel to the separating axis
    if(abs(startPos - targetPos) > 0.0001)
    {
      float f = (obstacleMin - 0.001 - startPos) / (targetPos - startPos);

      if(f > fraction)
        fraction = f;
    }
  }

  return fraction;
}

struct BoundingBox
{
  BoundingBox(Vec2f p) { min = max = p; }
  void add(Vec2f p)
  {
    min.x = std::min(min.x, p.x);
    max.x = std::max(max.x, p.x);
    min.y = std::min(min.y, p.y);
    max.y = std::max(max.y, p.y);
  }

  Vec2f min, max;
};
} // namespace

unique_ptr<IPhysics> createPhysics()
{
  return make_unique<Physics>();
}

bool ShapeBox::probe(AffineTransform transform, Box otherBox) const
{
  Box myBox;
  myBox.pos = NullVector;
  myBox.size = { transform.scale.x, transform.scale.y };
  return overlaps(myBox, otherBox);
}

float ShapeBox::raycast(AffineTransform transform, Box otherBox, Vec2f delta) const
{
  auto otherBoxHalfSize = Vec2f(otherBox.size.x, otherBox.size.y) * 0.5;
  auto obstacleHalfSize = transform.scale * 0.5;
  return ::raycastAgainstAABB(otherBox.pos + otherBoxHalfSize,
                              delta,
                              obstacleHalfSize,
                              obstacleHalfSize + otherBoxHalfSize);
}

bool ShapeTilemap::probe(AffineTransform /* transform */, Box otherBox) const
{
  auto const x1 = otherBox.pos.x;
  auto const y1 = otherBox.pos.y;
  auto const x2 = otherBox.pos.x + otherBox.size.x;
  auto const y2 = otherBox.pos.y + otherBox.size.y;

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

float ShapeTilemap::raycast(AffineTransform /* transform */, Box otherBox, Vec2f delta) const
{
  const auto boxSize = Vec2f(otherBox.size.x, otherBox.size.y);

  BoundingBox bb(otherBox.pos);

  bb.add(otherBox.pos);
  bb.add(otherBox.pos + boxSize);
  bb.add(delta + otherBox.pos);
  bb.add(delta + otherBox.pos + boxSize);

  auto const col1 = int(floor(bb.min.x));
  auto const col2 = int(floor(bb.max.x));
  auto const row1 = int(floor(bb.min.y));
  auto const row2 = int(floor(bb.max.y));

  const auto boxHalfSize = boxSize * 0.5;
  const auto tileHalfSize = UnitSize * 0.5;

  float fraction = 1;

  for(int row = row1; row <= row2; row++)
  {
    for(int col = col1; col <= col2; col++)
    {
      if(tiles->isInside(col, row) && tiles->get(col, row))
      {
        const auto tilePos = Vec2f(col, row) + tileHalfSize;
        float f = ::raycastAgainstAABB(otherBox.pos + boxHalfSize, delta, tilePos, boxHalfSize + tileHalfSize);

        if(f < fraction)
          fraction = f;
      }
    }
  }

  return fraction;
}

