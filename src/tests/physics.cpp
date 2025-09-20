/*
 * Copyright (C) 2021 - Sebastien Alaiwan
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
struct ToStringImpl<Vec2f>
{
  static std::string call(const Vec2f& val)
  {
    return "(" + std::to_string(val.x) + ", " + std::to_string(val.y) + ")";
  }
};

void assertNearlyEqualsFunc(Vec2f expected, Vec2f actual, const char* file, int line)
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

struct CornerShape : Shape
{
  bool probe(Box box) const override
  {
    return box.pos.y < 0 || box.pos.x < 0;
  }

  float raycast(Box box, Vec2f delta) const override
  {
    float fraction = 1;

    {
      float x1 = box.pos.x;
      float x2 = box.pos.x + delta.x;

      if(x1 > 0 && x2 < 0)
      {
        float fractionX = (0 - x1) / (x2 - x1);

        if(fractionX < fraction)
          fraction = fractionX;
      }
    }

    {
      float y1 = box.pos.y;
      float y2 = box.pos.y + delta.y;

      if(y1 > 0 && y2 < 0)
      {
        float fractionY = (0 - y1) / (y2 - y1);

        if(fractionY < fraction)
          fraction = fractionY;
      }
    }

    return fraction;
  }
};

struct Fixture
{
  Fixture() : physics(createPhysics())
  {
    walls.solid = true;
    walls.collisionGroup = 1;
    walls.shape = &cornerShape;
    walls.size = { 100, 100 };
    physics->addBody(&walls);
    walls.size = { 1, 1 };

    mover.collidesWith = 1;
    physics->addBody(&mover);
  }

  std::unique_ptr<IPhysics> physics;
  Body mover;
  Body walls;
  CornerShape cornerShape;
};

unittest("Physics: raycast")
{
  assertEquals(1.0f, raycastAgainstAABB({ 0, 0 }, { 1, 0 }, { 10, 10 }, { 1, 1 }));
  assertEquals(1.0f, raycastAgainstAABB({ -1, -0.9 }, { 2, 2 }, { 5, -5 }, { 5, 5 }));
  assertNearlyEquals(Vec2f(0.25, 0), Vec2f(raycastAgainstAABB({ 0, 0 }, { 8, 0 }, { 4, 0 }, { 2, 5 }), 0));
}

unittest("Physics: simple move")
{
  Fixture fix;
  fix.mover.pos = Vec2f(10, 10);

  auto allowed = fix.physics->moveBody(&fix.mover, Vec2f(10, 0));
  assert(allowed);
  assertNearlyEquals(Vec2f(20, 10), fix.mover.pos);
}

unittest("Physics: left move, blocked by vertical wall at x=0")
{
  Fixture fix;
  fix.mover.pos = Vec2f(10, 10);

  auto allowed = fix.physics->moveBody(&fix.mover, Vec2f(-20, 0));
  assert(allowed <= 0.5);

  assertNearlyEquals(Vec2f(0, 10), fix.mover.pos);
}

unittest("Physics: left move, blocked by a bigger body")
{
  Fixture fix;
  fix.mover.pos = Vec2f(100, 10);
  fix.mover.size = Vec2f(1, 1);

  Body blocker;
  blocker.pos = Vec2f(200, 5);
  blocker.size = Vec2f(8, 10);
  blocker.solid = true;
  fix.physics->addBody(&blocker);

  auto allowed = fix.physics->moveBody(&fix.mover, Vec2f(100, 0));
  assert(allowed < 1);

  assertNearlyEquals(Vec2f(199, 10), fix.mover.pos);
}

unittest("Physics: left move, blocked by a smaller body")
{
  Fixture fix;
  fix.mover.pos = Vec2f(100, 10);
  fix.mover.size = Vec2f(3, 3);

  Body blocker;
  blocker.pos = Vec2f(200, 11);
  blocker.size = Vec2f(1, 1);
  blocker.solid = true;
  fix.physics->addBody(&blocker);

  auto allowed = fix.physics->moveBody(&fix.mover, Vec2f(100, 0));
  assert(allowed < 1);

  assertNearlyEquals(Vec2f(197, 10), fix.mover.pos);
}

