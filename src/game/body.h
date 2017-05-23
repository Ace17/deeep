#include <functional>
#include "base/geom.h"

typedef GenericVector<float> Vector;
typedef GenericSize<float> Size;
typedef GenericBox<float> Box;
typedef GenericBox<int> IntBox;

static auto const UnitSize = Size(1, 1);

using namespace std;

static auto const PRECISION = 1024;
IntBox roundBox(Box b);

struct Body
{
  bool solid = false;
  bool pusher = false; // push and crush?
  Vector pos;
  Size size = Size(1, 1);
  int collisionGroup = 1;
  int collidesWith = 0xFFFF;
  Body* ground = nullptr; // the body we rest on (if any)

  // only called if (this->collidesWith & other->collisionGroup)
  virtual void onCollision(Body* /*other*/)
  {
  }

  Box getFBox() const
  {
    Box r;
    r.x = pos.x;
    r.y = pos.y;
    r.height = size.height;
    r.width = size.width;
    return r;
  }

  IntBox getBox() const
  {
    return roundBox(getFBox());
  }
};

struct IPhysicsProbe
{
  virtual bool moveBody(Body* body, Vector delta) = 0;
  virtual bool isSolid(const Body* body, IntBox) const = 0;
  virtual Body* getBodiesInBox(IntBox myBox, int collisionGroup, bool onlySolid = false, const Body* except = nullptr) const = 0;
};

struct IPhysics : IPhysicsProbe
{
  // called by game
  virtual void addBody(Body* body) = 0;
  virtual void removeBody(Body* body) = 0;
  virtual void clearBodies() = 0;
  virtual void checkForOverlaps() = 0;
  virtual void setEdifice(function<bool(IntBox)> isSolid) = 0;
};

