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

static
Rect2f enlarge(Rect2f rect, float ratio)
{
  Rect2f r;
  r.width = rect.width * ratio;
  r.height = rect.height * ratio;
  r.x = rect.x - (r.width - rect.width) * 0.5;
  r.y = rect.y - (r.height - rect.height) * 0.5;
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

  void clearBodies()
  {
    m_bodies.clear();
  }

  bool moveBody(Body* body, Vector2f delta)
  {
    auto rect = body->getRect();
    rect.x += delta.x;
    rect.y += delta.y;

    if(isSolid(rect))
      return false;

    body->pos += delta;
    return true;
  }

  bool isSolid(Rect2f rect) const
  {
    if(rectOverlapsSolidBody(rect))
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
      auto otherRect = enlarge(other.getRect(), 1.05);

      if(overlaps(rect, otherRect))
      {
        if(other.collidesWith & me.collisionGroup)
          other.onCollision(&me);

        if(me.collidesWith & other.collisionGroup)
          me.onCollision(&other);
      }
    }
  }

  void setEdifice(function<bool(Rect2f)> isSolid)
  {
    m_isSolid = isSolid;
  }

private:
  bool rectOverlapsSolidBody(Rect2f myRect) const
  {
    for(auto& body : m_bodies)
    {
      if(!body->solid)
        continue;

      auto rect = body->getRect();

      if(overlaps(rect, myRect))
        return true;
    }

    return false;
  }

  vector<Body*> m_bodies;
  function<bool(Rect2f)> m_isSolid;
};

unique_ptr<IPhysics> createPhysics()
{
  return make_unique<Physics>();
}

