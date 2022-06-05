#include "math.h"
#include <cmath> // sqrt

float dotProduct(Vec2f a, Vec2f b)
{
  return a.x * b.x + a.y * b.y;
}

Vec2f normalize(Vec2f a)
{
  auto const magnitude = sqrt(dotProduct(a, a));
  return a * 1.0 / magnitude;
}

