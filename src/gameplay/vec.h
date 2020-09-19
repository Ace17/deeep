#pragma once

#include "base/geom.h"

typedef GenericVector<float> Vector;
typedef GenericSize<float> Size;
typedef GenericBox<float> Box;
typedef GenericBox<int> IntBox;

static auto const UnitSize = Size(1, 1);
static auto const Up = GenericVector<float>(0, 1);
static auto const Down = GenericVector<float>(0, -1);
static auto const NullVector = GenericVector<float>(0, 0);

static auto const PRECISION = 1024; // fixed-point precision for collisions
// (if changing it, check the slow pushers (e.g elevators) still work)

