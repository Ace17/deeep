// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Basic geometric types: Vec2f, Vec2i

#pragma once

struct Vec2f
{
  float x = 0;
  float y = 0;

  Vec2f() = default;
  Vec2f(const Vec2f &) = default;
  Vec2f& operator = (const Vec2f&) = default;
  Vec2f(float x_, float y_)
    : x(x_)
    , y(y_)
  {
  }

  friend void operator += (Vec2f& a, Vec2f b) { a = a + b; }
  friend void operator -= (Vec2f& a, Vec2f b) { a = a - b; }
  friend void operator *= (Vec2f& a, float b) { a = a * b; }
  friend void operator /= (Vec2f& a, float b) { a = a / b; }

  friend Vec2f operator - (Vec2f v) { return Vec2f{ -v.x, -v.y }; }
  friend Vec2f operator + (Vec2f a, Vec2f b) { return Vec2f{ a.x + b.x, a.y + b.y }; }
  friend Vec2f operator - (Vec2f a, Vec2f b) { return Vec2f{ a.x - b.x, a.y - b.y }; }
  friend Vec2f operator * (Vec2f v, float f) { return Vec2f{ v.x* f, v.y* f }; }
  friend Vec2f operator * (float f, Vec2f v) { return v * f; }
  friend Vec2f operator / (Vec2f v, float f) { return Vec2f{ v.x / f, v.y / f }; }

  friend bool operator == (Vec2f a, Vec2f b) { return a.x == b.x && a.y == b.y; }
};

struct Vec2i
{
  int x = 0;
  int y = 0;

  Vec2i() = default;
  Vec2i(const Vec2i &) = default;
  Vec2i& operator = (const Vec2i&) = default;
  Vec2i(int x_, int y_)
    : x(x_)
    , y(y_)
  {
  }

  friend void operator += (Vec2i& a, Vec2i b) { a = a + b; }
  friend void operator -= (Vec2i& a, Vec2i b) { a = a - b; }
  friend void operator *= (Vec2i& a, int b) { a = a * b; }
  friend void operator /= (Vec2i& a, int b) { a = a / b; }

  friend Vec2i operator - (Vec2i v) { return Vec2i{ -v.x, -v.y }; }
  friend Vec2i operator + (Vec2i a, Vec2i b) { return Vec2i{ a.x + b.x, a.y + b.y }; }
  friend Vec2i operator - (Vec2i a, Vec2i b) { return Vec2i{ a.x - b.x, a.y - b.y }; }
  friend Vec2i operator * (Vec2i v, int f) { return Vec2i{ v.x* f, v.y* f }; }
  friend Vec2i operator * (int f, Vec2i v) { return v * f; }
  friend Vec2i operator / (Vec2i v, int f) { return Vec2i{ v.x / f, v.y / f }; }

  friend bool operator == (Vec2i a, Vec2i b) { return a.x == b.x && a.y == b.y; }
  friend bool operator != (Vec2i a, Vec2i b) { return !(a == b); }
};

