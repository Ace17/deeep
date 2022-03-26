// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "span.h"

struct String : Span<const char>
{
  constexpr String() = default;

  constexpr String(const char* data_, int len_)
  {
    data = data_;
    len = len_;
  }

  template<size_t N>
  constexpr String(const char (& stringLiteral)[N])
  {
    data = stringLiteral;
    len = N - 1; // remove NUL terminator
  }

  // construction from std::string
  template<typename U, typename = decltype(((U*)0)->c_str())>
  constexpr String(U const& s)
  {
    data = s.c_str();
    len = s.size();
  }
};

String format(Span<char> buf, const char* fmt, ...);

