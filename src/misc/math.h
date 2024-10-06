#pragma once
#include "base/geom.h"

float dotProduct(Vec2f a, Vec2f b);
Vec2f normalize(Vec2f a);

static const auto PI = 3.14159265358979323846;

template<typename T>
T lerp(T a, T b, float alpha)
{
  return a * (1 - alpha) + b * alpha;
}

