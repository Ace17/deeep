// Copyright (C) 2022 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Matrix types

#pragma once

#include <cassert>

#include "box.h"

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
    data = new T[size.x * size.y];

    for(int i = 0; i < size.x * size.y; ++i)
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
    for(int y = 0; y < size.y; y++)
      for(int x = 0; x < size.x; x++)
        f(x, y, get(x, y));
  }

  template<typename Lambda>
  void scan(Lambda f) const
  {
    for(int y = 0; y < size.y; y++)
      for(int x = 0; x < size.x; x++)
        f(x, y, get(x, y));
  }

  bool isInside(int x, int y) const
  {
    if(x < 0 || y < 0)
      return false;

    if(x >= size.x || y >= size.y)
      return false;

    return true;
  }

private:
  T* data = nullptr;

  int raster(int x, int y) const
  {
    return y * size.x + x;
  }
};

