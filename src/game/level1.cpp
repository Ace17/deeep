/**
 * @brief Level 1
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cassert>
#include "engine/geom.h"
#include "engine/util.h"
#include "game/game.h"
#include "game/entities/switch.h"
#include "game/entities/wheel.h"

const auto W = 32 * 2;
const auto H = 32 * 2;

static const char data_level1[H][W + 1] =
{
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
  "ZZ             ZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "ZZ                                 A                          ZZ",
  "ZZ             *                   A       *                  ZZ",
  "ZZZZZZ     ZZZZZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZ   ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ    ZZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZ   ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ    ZZZ",
  "XXXXXXZZZZZZZZZZZZZZZ       ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ    ZZZ",
  "X              ZZZZZZ            ZZZZZ           ZZZZ        ZZZ",
  "Xa                       Z        ZZ                       ZZZZZ",
  "XZXXXX           *     Z   Z      ZZ                      ZZ  ZZ",
  "XXXXXX     ZZZZZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ   ZZZZZZZZZZZZZZZZZZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ   XXXXXZZZZZZZZZZZZZZ",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXX",
  "XX                                              B            XXX",
  "XX  b                           *         XXX   B            XXX",
  "XX  Z  XXXXXXXXXXXXXXXXXX             XXXXXXXXXXX   *   XXX  XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  XXX",
  "XXXXXXXXXXXXXXXXXXX                                          XXX",
  "XXXXXXXXXXXXXXXXXXX                                          XXX",
  "XXXXXXXXXXXXXXXXXXX                                          XXX",
  "XXXXXXXXXXXXXXXXXXX                                          XXX",
  "XXXXX          XXXX  XXX       c    XXXX    d      XXXXXXXXXXXXX",
  "XXXXX          XXXX  XXXCCCCCCCCCCCCXXXXDDDDDDDDDDDXXXXXXXXXXXXX",
  "XXXXX                XXX            XXXX           XXXXXXXXXXXXX",
  "XXXXX                XXX            XXXX           XXXXXXXXXXXXX",
  "XXXXX          XXXX  XXX    *    *  XXXX   **      XXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXX  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXX                                          XXX",
  "XXXXXXXXXXXXXXXXXXX                                          XXX",
  "XXXXX                                                        XXX",
  "XXXXX                                                        XXX",
  "XXXXXXXZXXX        XXXXX   b        XXXX    a      XX        XXX",
  "XXXXXXXXXBBBBBBBBBBXXXXXBBBBBBBBBBBBXXXXAAAAAAAAAAAXX        XXX",
  "XXXXX    B        BXXXXXB          BXXXXA         AXX        XXX",
  "XXXXX    B        B     B          B    A         AXX        XXX",
  "XXXXX    B        B     B   *    * B    A  **     AXX        XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX   XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                  XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                  XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX                  XXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX     XXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX     XXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         XXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         XXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         XXXXXXXXXXXX",
  "XX            XXXX             XXXXXXXXXXXX         XXXXXXXXXXXX",
  "XX                             XXXXXXXXXXXX     XXXXXXXXXXXXXXXX",
  "XX                             XXXXXXXXXXXX     XXXXXXXXXXXXXXXX",
  "XX                                                  XXXXXXXXXXXX",
  "XX                                                  XXXXXXXXXXXX",
  "XX !          XXXX                                  XXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
};

void loadLevel(Matrix<int>& tiles, Vector2i& start, IGame* game, int /*number*/)
{
  auto data = data_level1;

  for(int y = 0; y < H; ++y)
    for(int x = 0; x < W; ++x)
    {
      int tile = 0;
      auto val = data[H - 1 - y][x];
      switch(val)
      {
      case ' ':
        tile = 0;
        break;
      case '!':
        tile = 0;
        start = Vector2i(x, y);
        break;
      case '.': tile = 1;
        break;
      case 'X': tile = 5;
        break;
      case 'O': tile = 3;
        break;
      case 'Z': tile = 4;
        break;
      case 'a':
      case 'b':
      case 'c':
      case 'd':
        {
          auto sw = new Switch(val - 'a');
          sw->pos = Vector2f(x + 0.3, y + 0.15);
          game->spawn(sw);
          break;
        }
      case 'A':
      case 'B':
      case 'C':
      case 'D':
        {
          auto sw = new Door(val - 'A', game);
          sw->pos = Vector2f(x + 0.5, y + 0.5);
          game->spawn(sw);
          break;
        }
      case '*':
        {
          auto wh = new Wheel;
          wh->pos = Vector2f(x, y);
          game->spawn(wh);
          break;
        }
      }

      tiles.set(x, y, tile);
    }
}

