#include "math.h"
#include <cmath> // sqrt

float dotProduct(Vector2f a, Vector2f b)
{
  return a.x * b.x + a.y * b.y;
}

Vector2f normalize(Vector2f a)
{
  auto const magnitude = sqrt(dotProduct(a, a));
  return a * 1.0 / magnitude;
}

