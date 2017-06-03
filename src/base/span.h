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

