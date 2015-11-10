/**
 * @brief Basic geometric types
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Initialized
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct Initialized
{
  Initialized()
  {
    val = T(0);
  }

  operator T () const
  {
    return val;
  }

  T operator = (T const& val_)
  {
    val = val_;
    return val_;
  }

  T operator += (T const& val_)
  {
    val += val_;
    return val_;
  }

  T val;
};

typedef Initialized<int> Int;
typedef Initialized<bool> Bool;

///////////////////////////////////////////////////////////////////////////////
// Dimension
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct GenericDimension
{
  GenericDimension() : width(0), height(0)
  {
  }

  GenericDimension(T w, T h) : width(w), height(h)
  {
  }

  T width, height;
};

typedef GenericDimension<int> Dimension2i;
typedef GenericDimension<float> Dimension2f;

///////////////////////////////////////////////////////////////////////////////
// Vector
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct GenericVector
{
  typedef GenericVector<T> MyType;

  GenericVector() : x(0), y(0)
  {
  }

  GenericVector(T x_, T y_) : x(x_), y(y_)
  {
  }

  MyType operator += (MyType const& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  friend MyType operator + (MyType const& a, MyType const& b)
  {
    MyType r = a;
    r += b;
    return r;
  }

  T x, y;
};

typedef GenericVector<int> Vector2i;
typedef GenericVector<float> Vector2f;

///////////////////////////////////////////////////////////////////////////////
// Rect
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct GenericRect : GenericVector<T>, GenericDimension<T>
{
  GenericRect()
  {
  }

  GenericRect(T x, T y, T w, T h) :
    GenericVector<T>(x, y),
    GenericDimension<T>(w, h)
  {
  }
};

typedef GenericRect<int> Rect2i;
typedef GenericRect<float> Rect2f;

#include <algorithm>
#include <cassert>

template<typename T>
bool segmentsOverlap(T s1x1, T s1x2, T s2x1, T s2x2)
{
  if(s1x1 > s2x1)
  {
    std::swap(s1x1, s2x1);
    std::swap(s1x2, s2x2);
  }

  return s2x1 >= s1x1 && s2x1 <= s1x2;
}

template<typename T>
bool overlaps(GenericRect<T> const& a, GenericRect<T> const& b)
{
  assert(a.width >= 0);
  assert(a.height >= 0);
  assert(b.width >= 0);
  assert(b.height >= 0);

  if(!segmentsOverlap(a.x, a.x + a.width, b.x, b.x + b.width))
    return false;

  if(!segmentsOverlap(a.y, a.y + a.height, b.y, b.y + b.height))
    return false;

  return true;
}

