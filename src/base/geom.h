// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Basic geometric types

#pragma once

#include <cassert>

///////////////////////////////////////////////////////////////////////////////
// Dimension
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct GenericSize
{
  typedef GenericSize<T> MyType;

  GenericSize() : width(0), height(0)
  {
  }

  GenericSize(T w, T h) : width(w), height(h)
  {
  }

  template<typename F>
  friend MyType operator * (MyType const& a, F val)
  {
    return MyType(a.width * val, a.height * val);
  }

  template<typename F>
  friend MyType operator / (MyType const& a, F val)
  {
    return MyType(a.width / val, a.height / val);
  }

  bool operator == (GenericSize const& other) const
  {
    return width == other.width && height == other.height;
  }

  bool operator != (GenericSize const& other) const
  {
    return !(*this == other);
  }

  T width, height;
};

using Size2f = GenericSize<float>;
using Size2i = GenericSize<int>;

///////////////////////////////////////////////////////////////////////////////
// Vector
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct GenericVector
{
  typedef GenericVector<T> MyType;

  GenericVector() : x(0), y(0) {}
  GenericVector(T x_, T y_) : x(x_), y(y_) {}
  GenericVector(GenericSize<T> size) : x(size.width), y(size.height) {}

  MyType operator += (MyType const& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  MyType operator -= (MyType const& other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  template<typename F>
  friend MyType operator * (MyType const& a, F val)
  {
    return MyType(a.x * val, a.y * val);
  }

  template<typename F>
  friend MyType operator / (MyType const& a, F val)
  {
    return MyType(a.x / val, a.y / val);
  }

  template<typename F>
  friend MyType operator * (F val, MyType const& a)
  {
    return MyType(a.x * val, a.y * val);
  }

  friend MyType operator + (MyType const& a, MyType const& b)
  {
    MyType r = a;
    r += b;
    return r;
  }

  friend MyType operator - (MyType const& a, MyType const& b)
  {
    MyType r = a;
    r -= b;
    return r;
  }

  T x, y;
};

using Vec2f = GenericVector<float>;
using Vec2i = GenericVector<int>;

