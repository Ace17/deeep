// Copyright (C) 2018 - Sebastien Alaiwan
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

typedef GenericSize<int> Size2i;
typedef GenericSize<float> Size2f;

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

typedef GenericVector<int> Vector2i;
typedef GenericVector<float> Vector2f;

///////////////////////////////////////////////////////////////////////////////
// Box
///////////////////////////////////////////////////////////////////////////////

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
  auto swap = [] (T& a, T& b)
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
  assert(a.size.width >= 0);
  assert(a.size.height >= 0);
  assert(b.size.width >= 0);
  assert(b.size.height >= 0);

  if(!segmentsOverlap(a.pos.x, a.pos.x + a.size.width, b.pos.x, b.pos.x + b.size.width))
    return false;

  if(!segmentsOverlap(a.pos.y, a.pos.y + a.size.height, b.pos.y, b.pos.y + b.size.height))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Matrix
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct Matrix2
{
  Matrix2() = default;

  Matrix2(Matrix2 const &) = delete;
  void operator = (Matrix2 const&) = delete;

  Matrix2(Matrix2&& other)
  {
    data = other.data;
    size = other.size;
    other.data = nullptr;
  }

  void operator = (Matrix2&& other)
  {
    delete[] data;
    data = other.data;
    size = other.size;
    other.data = nullptr;
  }

  Matrix2(Size2i size_) : size(size_)
  {
    resize(size_);
  }

  ~Matrix2()
  {
    delete[] data;
  }

  void resize(Size2i size_)
  {
    delete[] data;

    size = size_;
    data = new T[size.width * size.height];

    for(int i = 0; i < size.width * size.height; ++i)
      data[i] = T();
  }

  Size2i size = Size2i(0, 0);

  T & get(int x, int y)
  {
    assert(isInside(x, y));
    return data[raster(x, y)];
  }

  const T & get(int x, int y) const
  {
    assert(isInside(x, y));
    return data[raster(x, y)];
  }

  void set(int x, int y, T const& val)
  {
    assert(isInside(x, y));
    get(x, y) = val;
  }

  template<typename Lambda>
  void scan(Lambda f)
  {
    for(int y = 0; y < size.height; y++)
      for(int x = 0; x < size.width; x++)
        f(x, y, get(x, y));
  }

  template<typename Lambda>
  void scan(Lambda f) const
  {
    for(int y = 0; y < size.height; y++)
      for(int x = 0; x < size.width; x++)
        f(x, y, get(x, y));
  }

  bool isInside(int x, int y) const
  {
    if(x < 0 || y < 0)
      return false;

    if(x >= size.width || y >= size.height)
      return false;

    return true;
  }

private:
  T* data = nullptr;

  int raster(int x, int y) const
  {
    return y * size.width + x;
  }
};

