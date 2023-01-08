#pragma once

#include "base/box.h"
#include "base/geom.h"

typedef Vec2f Vector;
typedef Vec2f Size;
typedef Rect2f Box;
typedef Rect2i IntBox;

static auto const UnitSize = Size(1, 1);
static auto const Up = Vec2f(0, 1);
static auto const Down = Vec2f(0, -1);
static auto const NullVector = Vec2f(0, 0);

static auto const PRECISION = 1024; // fixed-point precision for collisions
// (if changing it, check the slow pushers (e.g elevators) still work)

