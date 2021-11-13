#include "matrix3.h"

#include <cmath> // sin, cos

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

Matrix3f scale(Vector2f v)
{
  Matrix3f r(0);
  r[0][0] = v.x;
  r[1][1] = v.y;
  r[2][2] = 1;
  return r;
}

