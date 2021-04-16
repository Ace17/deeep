// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Matrix 3x3 class for display

#pragma once

struct Matrix3f
{
  Matrix3f(float init)
  {
    for(int row = 0; row < 3; ++row)
      for(int col = 0; col < 3; ++col)
        m_rows[row][col] = init;
  }

  struct row
  {
    float elements[3];

    operator float* () { return elements; }
    operator const float* () const { return elements; }
  };

  operator row* () { return m_rows; }
  operator const row* () const { return m_rows; }

  row m_rows[3];
};

inline
Matrix3f operator * (Matrix3f const& A, Matrix3f const& B)
{
  Matrix3f r(0);

  for(int row = 0; row < 3; ++row)
    for(int col = 0; col < 3; ++col)
    {
      double sum = 0;

      for(int k = 0; k < 3; ++k)
        sum += A.m_rows[row][k] * B.m_rows[k][col];

      r.m_rows[row][col] = sum;
    }

  return r;
}

inline
Matrix3f translate(Vector2f v)
{
  Matrix3f r(0);
  r[0][0] = 1;
  r[0][1] = 0;
  r[0][2] = v.x;
  r[1][0] = 0;
  r[1][1] = 1;
  r[1][2] = v.y;
  r[2][0] = 0;
  r[2][1] = 0;
  r[2][2] = 1;
  return r;
}

inline
Matrix3f rotate(float angle)
{
  auto const ca = cos(angle);
  auto const sa = sin(angle);

  Matrix3f r(0);
  r[0][0] = ca;
  r[0][1] = -sa;
  r[1][0] = sa;
  r[1][1] = ca;
  r[2][2] = 1;
  return r;
}

inline
Matrix3f scale(Vector2f v)
{
  Matrix3f r(0);
  r[0][0] = v.x;
  r[1][1] = v.y;
  r[2][2] = 1;
  return r;
}

