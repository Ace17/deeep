#include "game/body.h"
#include "base/util.h"
#include <vector>
#include <memory>

using namespace std;

template<typename T, typename Lambda>
void unstableRemove(vector<T>& container, Lambda predicate)
{
  for(int i = 0; i < (int)container.size(); ++i)
  {
    if(predicate(container[i]))
    {
      auto const j = (int)container.size() - 1;

      swap(container[i], container[j]);

      if(i != j)
        --i;

      container.pop_back();
    }
  }
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

  void clearBodies()
  {
    m_bodies.clear();
  }

  bool moveBody(Body* body, Vector2f delta)
  {
    auto rect = body->getRect();
    rect.x += delta.x;
    rect.y += delta.y;

    auto const blocked = isSolid(body, rect);

    if(blocked)
    {
      if(auto blocker = getSolidBodyInRect(rect, body))
        collideBodies(*body, *blocker);
    }
    else
    {
      body->pos += delta;

      if(body->pusher)
      {
        // move stacked bodies
        for(auto otherBody : m_bodies)
        {
          if(otherBody->ground == body)
            moveBody(otherBody, delta);
        }

        // push potential non-solid bodies
        for(auto other : m_bodies)
          if(other != body && overlaps(rect, other->getRect()))
            moveBody(other, delta);
      }
    }

    // update ground
    if(!body->pusher)
    {
      auto feet = rect;
      feet.y -= 0.01;
      feet.height = 0.01;
      body->ground = getSolidBodyInRect(feet, body);
    }

    return !blocked;
  }

  bool isSolid(const Body* except, Rect2f rect) const
  {
    if(getSolidBodyInRect(rect, except))
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

      auto rect = me.getRect();
      auto otherRect = other.getRect();

      if(overlaps(rect, otherRect))
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

  void setEdifice(function<bool(Rect2f)> isSolid)
  {
    m_isSolid = isSolid;
  }

  Body* getBodiesInRect(Rect2f myRect, int collisionGroup, bool onlySolid, const Body* except) const
  {
    for(auto& body : m_bodies)
    {
      if(onlySolid && !body->solid)
        continue;

      if(body == except)
        continue;

      if(!(body->collisionGroup & collisionGroup))
        continue;

      auto rect = body->getRect();

      if(overlaps(rect, myRect))
        return body;
    }

    return nullptr;
  }

private:
  Body* getSolidBodyInRect(Rect2f myRect, const Body* except) const
  {
    return getBodiesInRect(myRect, -1, true, except);
  }

  vector<Body*> m_bodies;
  function<bool(Rect2f)> m_isSolid;
};

unique_ptr<IPhysics> createPhysics()
{
  return make_unique<Physics>();
}

