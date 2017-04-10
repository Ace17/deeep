#include "tests/tests.h"
#include "game/body.h"
#include <cmath>
#include <memory>

///////////////////////////////////////////////////////////////////////////////

#define assertNearlyEquals(u, v) \
  assertNearlyEqualsFunc(u, v, __FILE__, __LINE__)

static inline
std::ostream & operator << (std::ostream& o, const Vector2f& v)
{
  o << "(";
  o << v.x;
  o << ", ";
  o << v.y;
  o << ")";
  return o;
}

void assertNearlyEqualsFunc(Vector2f expected, Vector2f actual, const char* file, int line)
{
  auto delta = expected - actual;

  if(fabs(delta.x) > 0.01 || fabs(delta.y) > 0.01)
  {
    using namespace std;
    stringstream ss;
    ss << "Assertion failure" << endl;
    ss << file << "(" << line << ")" << endl;
    ss << "Expected '" << expected << "', got '" << actual << "'" << endl;
    throw logic_error(ss.str());
  }
}

///////////////////////////////////////////////////////////////////////////////

unique_ptr<IPhysics> createPhysics();

static
bool isSolid(Rect2f rect)
{
  return rect.y < 0 || rect.x < 0;
}

unittest("Physics: simple move")
{
  auto physics = createPhysics();
  physics->setEdifice(&isSolid);

  Body mover;
  mover.pos = Vector2f(10, 10);

  physics->addBody(&mover);

  auto allowed = physics->moveBody(&mover, Vector2f(10, 0));
  assert(allowed);
  assertNearlyEquals(Vector2f(20, 10), mover.pos);
}

unittest("Physics: left move, blocked by vertical wall at (0;_)")
{
  auto physics = createPhysics();
  physics->setEdifice(&isSolid);

  Body mover;
  mover.pos = Vector2f(10, 10);

  physics->addBody(&mover);

  auto allowed = physics->moveBody(&mover, Vector2f(-20, 0));
  assert(!allowed);

  assertNearlyEquals(Vector2f(10, 10), mover.pos);
}

unittest("Physics: left move, blocked by a bigger body")
{
  auto physics = createPhysics();
  physics->setEdifice(&isSolid);

  Body mover;
  mover.pos = Vector2f(100, 10);
  mover.size = Size2f(1, 1);
  physics->addBody(&mover);

  Body blocker;
  blocker.pos = Vector2f(200, 5);
  blocker.size = Size2f(10, 10);
  blocker.solid = true;
  physics->addBody(&blocker);

  auto allowed = physics->moveBody(&mover, Vector2f(100, 0));
  assert(!allowed);

  assertNearlyEquals(Vector2f(100, 10), mover.pos);
}

