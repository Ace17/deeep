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
  bool probe(AffineTransform /* transform */, Box rect) const override
  {
    return rect.pos.y < 0 || rect.pos.x < 0;
  }

  float raycast(AffineTransform /* transform */, Box otherBox, Vec2f delta) const override
  {
    float fraction = 1;

    {
      float x1 = otherBox.pos.x;
      float x2 = otherBox.pos.x + delta.x;

      if(x1 > 0 && x2 < 0)
      {
        float fractionX = (0 - x1) / (x2 - x1);

        if(fractionX < fraction)
          fraction = fractionX;
      }
    }

    {
      float y1 = otherBox.pos.y;
      float y2 = otherBox.pos.y + delta.y;

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
    physics->addBody(&walls);

    mover.collidesWith = 1;
    physics->addBody(&mover);
  }

  unique_ptr<IPhysics> physics;
  Body mover;
  Body walls;
  CornerShape cornerShape;
};

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
  fix.mover.size = Size2f(1, 1);

  Body blocker;
  blocker.pos = Vec2f(200, 5);
  blocker.size = Size2f(10, 10);
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
  fix.mover.size = Size2f(3, 3);

  Body blocker;
  blocker.pos = Vec2f(200, 11);
  blocker.size = Size2f(1, 1);
  blocker.solid = true;
  fix.physics->addBody(&blocker);

  auto allowed = fix.physics->moveBody(&fix.mover, Vec2f(100, 0));
  assert(allowed < 1);

  assertNearlyEquals(Vec2f(197, 10), fix.mover.pos);
}

