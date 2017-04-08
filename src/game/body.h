#include <functional>
#include "base/geom.h"
#include "game.h"

using namespace std;

struct Body
{
  bool solid = false;
  Vector2f pos;
  Size2f size = Size2f(1, 1);
  int collisionGroup = 1;
  int collidesWith = 0xFFFF;

  // only called if (this->collidesWith & other->collisionGroup)
  virtual void onCollision(Body* /*other*/)
  {
  }

  Rect2f getRect() const
  {
    Rect2f r;
    r.x = pos.x;
    r.y = pos.y;
    r.height = size.height;
    r.width = size.width;
    return r;
  }
};

struct IPhysics
{
  virtual void addBody(Body* body) = 0;
  virtual void removeBody(Body* body) = 0;
  virtual void clearBodies() = 0;
  virtual bool moveBody(Body* body, Vector2f delta) = 0;
  virtual void checkForOverlaps() = 0;
  virtual void setEdifice(function<bool(Rect2f)> isSolid) = 0;
};

