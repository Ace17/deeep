/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <stddef.h>

template<typename T>
struct Span
{
  T* data;
  int len;

  T* begin() const
  {
    return data;
  }

  T* end() const
  {
    return data + len;
  }
};

template<typename T, size_t N>
Span<T> makeSpan(T(&tab)[N])
{
  Span<T> r;
  r.data = tab;
  r.len = N;
  return r;
}

