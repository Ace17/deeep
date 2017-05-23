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
bool isSolid(IntBox rect)
{
  return rect.y < 0 || rect.x < 0;
}

struct Fixture
{
  Fixture() : physics(createPhysics())
  {
    physics->setEdifice(&isSolid);
    physics->addBody(&mover);
  }

  unique_ptr<IPhysics> physics;
  Body mover;
};

unittest("Physics: simple move")
{
  Fixture fix;
  fix.mover.pos = Vector2f(10, 10);

  auto allowed = fix.physics->moveBody(&fix.mover, Vector2f(10, 0));
  assert(allowed);
  assertNearlyEquals(Vector2f(20, 10), fix.mover.pos);
}

unittest("Physics: left move, blocked by vertical wall at x=0")
{
  Fixture fix;
  fix.mover.pos = Vector2f(10, 10);

  auto allowed = fix.physics->moveBody(&fix.mover, Vector2f(-20, 0));
  assert(!allowed);

  assertNearlyEquals(Vector2f(10, 10), fix.mover.pos);
}

unittest("Physics: left move, blocked by a bigger body")
{
  Fixture fix;
  fix.mover.pos = Vector2f(100, 10);
  fix.mover.size = Size2f(1, 1);

  Body blocker;
  blocker.pos = Vector2f(200, 5);
  blocker.size = Size2f(10, 10);
  blocker.solid = true;
  fix.physics->addBody(&blocker);

  auto allowed = fix.physics->moveBody(&fix.mover, Vector2f(100, 0));
  assert(!allowed);

  assertNearlyEquals(Vector2f(100, 10), fix.mover.pos);
}

