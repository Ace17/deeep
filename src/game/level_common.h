#pragma once

#include "game/game.h"
#include "room.h"

int interpretTile(Vector2i pos, Vector2i& start, IGame* game, int val);

Room loadLevel(Matrix<char> const& input, IGame* game);

template<size_t W_plus_one, size_t H>
Matrix<char> toMatrix(const char(&data)[H][W_plus_one])
{
  auto const W = W_plus_one - 1;

  auto r = Matrix<char>(Size2i(W, H));

  for(size_t y = 0; y < H; ++y)
  {
    for(size_t x = 0; x < W; ++x)
    {
      auto val = data[H - 1 - y][x];
      r.set(x, y, val);
    }
  }

  return r;
}

