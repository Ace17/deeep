// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <stddef.h>

template<typename T>
struct Span
{
  T* data = nullptr;
  int len = 0;

  Span() = default;
  Span(T* tab, int N) : data(tab), len(N) {}

  template<size_t N>
  Span(T (& tab)[N]) : data(tab), len(N) {}

  // construction from vector/string
  template<typename U, typename = decltype(((U*)0)->data())>
  Span(U& s)
  {
    data = s.data();
    len = s.size();
  }

  operator Span<const T>() { return { data, len }; }

  T* begin() const { return data; }
  T* end() const { return data + len; }
  T& operator [] (int i) { return data[i]; }
  void operator += (int i) { data += i; len -= i; }
};

