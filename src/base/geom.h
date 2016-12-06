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

#include <algorithm>
#include <cassert>

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

  operator T const & () const
  {
    return val;
  }

  operator T & ()
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
struct GenericSize
{
  GenericSize() : width(0), height(0)
  {
  }

  GenericSize(T w, T h) : width(w), height(h)
  {
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
// Rect
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct GenericRect : GenericVector<T>, GenericSize<T>
{
  GenericRect()
  {
  }

  GenericRect(T x, T y, T w, T h) :
    GenericVector<T>(x, y),
    GenericSize<T>(w, h)
  {
  }
};

typedef GenericRect<int> Rect2i;
typedef GenericRect<float> Rect2f;

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

///////////////////////////////////////////////////////////////////////////////
// Matrix
///////////////////////////////////////////////////////////////////////////////

template<typename T>
struct Matrix
{
  Matrix(Size2i size_) : size(size_)
  {
    data = nullptr;
    resize(size_);
  }

  ~Matrix()
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

  Size2i size;

  T& get(int x, int y)
  {
    assert(isInside(x, y));
    return data[x + y * size.width];
  }

  const T& get(int x, int y) const
  {
    assert(isInside(x, y));
    return data[x + y * size.width];
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
  T* data;
};

