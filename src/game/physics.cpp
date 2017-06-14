#include "game/body.h"
#include "base/util.h"
#include "physics.h"
#include <vector>
#include <memory>

using namespace std;

int roundCoord(float x)
{
  return round((double)x * PRECISION);
}

IntBox roundBox(Box b)
{
  IntBox r;
  r.x = roundCoord(b.x);
  r.y = roundCoord(b.y);
  r.width = roundCoord(b.width);
  r.height = roundCoord(b.height);
  return r;
}

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
    auto newPos = body->pos + delta;

    auto frect = body->getFBox();
    frect.pos() = newPos;

    auto rect = roundBox(frect);

    auto const blocked = isSolid(body, rect);

    if(blocked)
    {
      if(auto blocker = getSolidBodyInBox(rect, body))
        collideBodies(*body, *blocker);
    }
    else
    {
      if(body->pusher)
        pushOthers(body, rect, delta);

      body->pos = frect.pos();
      // assert(!getSolidBodyInBox(body->getBox(), body));
    }

    // update ground
    if(!body->pusher)
    {
      auto feet = body->getBox();
      feet.height = 16;
      feet.y -= feet.height;
      body->ground = getSolidBodyInBox(feet, body);
    }

    return !blocked;
  }

  void pushOthers(Body* body, IntBox rect, Vector delta)
  {
    // move stacked bodies
    for(auto otherBody : m_bodies)
    {
      if(otherBody->ground == body)
        moveBody(otherBody, delta);
    }

    // push potential non-solid bodies
    for(auto other : m_bodies)
      if(other != body && overlaps(rect, other->getBox()))
        moveBody(other, delta);
  }

  bool isSolid(const Body* except, IntBox rect) const
  {
    if(getSolidBodyInBox(rect, except))
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

  void setEdifice(function<bool(IntBox)> edifice)
  {
    m_isSolid = edifice;
  }

  Body* getBodiesInBox(IntBox myBox, int collisionGroup, bool onlySolid, const Body* except) const
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
  Body* getSolidBodyInBox(IntBox myBox, const Body* except) const
  {
    return getBodiesInBox(myBox, -1, true, except);
  }

  vector<Body*> m_bodies;
  function<bool(IntBox)> m_isSolid;
};

unique_ptr<IPhysics> createPhysics()
{
  return make_unique<Physics>();
}

