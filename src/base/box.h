// Copyright (C) 2022 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <cassert>

#include "geom.h"

template<typename T>
struct GenericBox
{
  GenericBox() = default;
  GenericBox(T x, T y, T w, T h) : pos(x, y), size(w, h) {}
  GenericBox(GenericVector<T> pos_, GenericSize<T> size_) : pos(pos_), size(size_) {}

  GenericVector<T> pos;
  GenericSize<T> size;
};

typedef GenericBox<int> Rect2i;
typedef GenericBox<float> Rect2f;

template<typename T>
bool segmentsOverlap(T a_left, T a_right, T b_left, T b_right)
{
  static auto swap = [] (T& a, T& b)
    {
      auto t = a;
      a = b;
      b = t;
    };

  if(a_left > b_left)
  {
    swap(a_left, b_left);
    swap(a_right, b_right);
  }

  return b_left >= a_left && b_left < a_right;
}

template<typename T>
bool overlaps(GenericBox<T> const& a, GenericBox<T> const& b)
{
  assert(a.size.x >= 0);
  assert(a.size.y >= 0);
  assert(b.size.x >= 0);
  assert(b.size.y >= 0);

  if(!segmentsOverlap(a.pos.x, a.pos.x + a.size.x, b.pos.x, b.pos.x + b.size.x))
    return false;

  if(!segmentsOverlap(a.pos.y, a.pos.y + a.size.y, b.pos.y, b.pos.y + b.size.y))
    return false;

  return true;
}

