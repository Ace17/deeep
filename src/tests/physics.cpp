/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "gameplay/body.h"
#include "gameplay/physics.h"
#include "tests.h"
#include <cmath>
#include <memory>

///////////////////////////////////////////////////////////////////////////////

#define assertNearlyEquals(u, v) \
  assertNearlyEqualsFunc(u, v, __FILE__, __LINE__)

template<>
struct ToStringImpl<Vector2f>
{
  static std::string call(const Vector2f& val)
  {
    return "(" + std::to_string(val.x) + ", " + std::to_string(val.y) + ")";
  }
};

void assertNearlyEqualsFunc(Vector2f expected, Vector2f actual, const char* file, int line)
{
  auto delta = expected - actual;

  if(fabs(delta.x) > 0.01 || fabs(delta.y) > 0.01)
  {
    std::string ss;
    ss += "Assertion failure\n";
    ss += std::string(file) + "(" + std::to_string(line) + ")\n";
    ss += "Expected '" + testValueToString(expected) + "', got '" + testValueToString(actual) + "'\n";
    fprintf(stderr, "%s", ss.c_str());
    abort();
  }
}

///////////////////////////////////////////////////////////////////////////////

static
bool isSolid(IntBox rect)
{
  return rect.pos.y < 0 || rect.pos.x < 0;
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

