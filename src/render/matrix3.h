// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Matrix 3x3 class for display

#pragma once

#include "base/geom.h"

struct Matrix3f
{
  Matrix3f() = default;

  Matrix3f(float init)
  {
    for(int row = 0; row < 3; ++row)
      for(int col = 0; col < 3; ++col)
        m_rows[row][col] = init;
  }

  struct row
  {
    float elements[3 + 1] {}; // +1: slight speedup

    operator float* ()
    {
      return elements;
    }
    operator const float* () const
    {
      return elements;
    }
  };

  operator row* ()
  {
    return m_rows;
  }
  operator const row* () const
  {
    return m_rows;
  }

  row m_rows[3] {};
};

Matrix3f operator * (Matrix3f const& A, Matrix3f const& B);

Matrix3f computeTransform(Vec2f pos, float angle, Vec2f scale);
Matrix3f translate(Vec2f v);
Matrix3f rotate(float angle);
Matrix3f scale(Vec2f v);

