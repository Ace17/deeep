// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
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
  Span(T(&tab)[N]) : data(tab), len(N) {}

  T* begin() const { return data; }
  T* end() const { return data + len; }
};

