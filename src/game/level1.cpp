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
  "................................................................",
  "...       .........................       ......................",
  "...                                                           ..",
  "...    !           ..........  ....       .......  .......... ..",
  "............       .........   ..................  .......... ..",
  "............       .........  ..............   A   .X.......  ..",
  ".       ....   ... .........   ..       ....   ... .........   .",
  ".              ...     ......  ..              ... ..........  .",
  ".           ..............     ..           ..............     .",
  ".       ..................    ...       ..................    ..",
  ".  ..........        ........ ...  ..........        ........ ..",
  ". ........... ...... ........ ...  .......... ...... ........ ..",
  "............. ...... ............ ........... ...... ........ ..",
  ".             ......          ... ..          ......          ..",
  "..           ................      .         ...................",
  ".. ......................................... ...................",
  ".. .........................................            a ......",
  "..        ...................             .........XXXXXXXXXXXXX",
  "XXX                           XXXXX                           XX",
  "XXX                XXXXXXXXXX XXXXX                XXXXXXXXXX XX",
  "XXXXXXXXXXXXXXXXXXBXXXXXXXXXX XXXXXXXXXXXXXX       XXXXXXXXXX XX",
  "XXXXXXXXXXXX       XZXXXXXXX  XXXXXXXXXXXXXX       XXXXXXXXX  XX",
  "X       XXXX   XXX XXXXXXXXX   XXXXXXXXXXXXX   XXX XXXXXXXXX   X",
  "X              XXX XXXXXXXXXX  XXXXXXXXX       XXX XXXXXXXXXX  X",
  "X           XXXXXXXXXXXXXX     XXXXXXXXX    XXXXXXXXXXX        X",
  "X       XXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXX XX    XX",
  "X  XXXXXXXXXX        XXXXXXXXXXXXXXXXXXXXXXXX        XX XXXXX XX",
  "X  XXXXXXXXXX XXXXXX XXXXXXXX            XXXX XXXXXX XX       bX",
  "X XXXXXXXXXXX XXXXXX XXXXXXXX            XXXX XXXXXX XXXXXXXXXXX",
  "X             XXXXXX          XXXXXXXXXX      XZXXXX           X",
  "XXX          XXXXXXXXXXXXXXXXXXXXXXXXXXX     XXXXXXXXXXXXXXX  XX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXXXX   XX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX          XXXXXXXXXXXXXXXXX   XXX",
  "XX    cXXXXXXXXX                XXXXXXXXXXXXXXXXXXXXXXXXX   XXXX",
  "XX  XXXXXXXXXXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXXX   XXXXX",
  "ZZ  ZZZ          ZZZZZZZZZZZZZZZZZZZZZZXXXXXXXXXX  XXXX    XXXXX",
  "ZZ ZZZZ ZZZZZZZZZZZZZZZ       ZZZZZZZZZ         C       ZZZZZZZZ",
  "ZZ      ZZZZZZZZZZZZZZZ                         C       ZZZZZZZZ",
  "ZZZZZZZZZZZZZZZZZ                        ZZZZZZZZZZZZZZZZZZZZZZZ",
  "ZZZZZZZZZZ                    ZZZZZZZZZ  ZZZ       Z      Z    Z",
  "ZZZZ             ZZZZZZZZZZZZZZZZZZZZZZ                        Z",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ                        Z",
  "ZZZZZZZZZZ       ZZZZZZZZZZZZZZZZZZZZZZZZZZZ       Z      Z    Z",
  "ZZZZZZZZZZ               Z      ZZZZZZZZZZZZZZZZZZZZZZ  ZZZZZZZZ",
  "ZZZZZZZZZZ       ZZ             ZZZZZZZZZZZZZZZZZZZZ      ZZZZZZ",
  "ZZZZZZ       ZZ  ZZ      Z           ZZ                   ZZZZZZ",
  "ZZZZZZ       ZZ  ZZ      Z      XXX  ZZ  ZZZZZZZZZZZ      ZZZZZZ",
  "ZZZZZZ           ZZZZ  ZZZZZZZZZXXXZ ZZ  ZZZZZZZZZZZZZZZZZZZZZZZ",
  "ZZZZZZ           ZZZZ          ZZZZZ ZZ  ZZ                ZZZZZ",
  "ZZZZZZ  ZZZZZZZZZZZZZZZZZZZZZZ ZZZZZ     ZZ    ZZ ZZZZZZZZ ZZZZZ",
  "ZZZZZZ  ZZZZZZZZZZZZZ       ZZ ZZZZZZZZZZZZZZZZZZ ZZZZZZ   ZZZZZ",
  "ZZ               ZZZZ       ZZ ZZZZZZZZZZZ        ZZZZZZ  ZZZZZZ",
  "ZZ ZZZZZZZZZZZZZZZZZZ          ZZZZZZZZ        ZZZZZ       ZZZZZ",
  "ZZ ZZZZZZZZZZZ     D        ZZZZZZZZZZZ    ZZZZZZZZ    ZZZ    ZZ",
  "ZZ       ZZZZZ  ZZZZZ       ZZZZZZZZZZZ  Z ZZZZZ       ZZZ    ZZ",
  "ZZZ    d ZZZZZ  ZZZZZZZZZZZZZZZZZZZZ       ZZZZZ       ZZZ    ZZ",
  "ZZZZZZZXZZZZ      ZZZ     ZZZZ       ZZZZZZZZ      ZZZZZZZ ZZZZZ",
  "ZZZZZZZZZZZZ  ZZ      ZZZ       ZZZZZZZZZZZZZ ZZZZZZZZZZZZ ZZZZZ",
  "ZZZZ          ZZ  ZZZZZZZZZZZZZZZZZZZZZZZZ            ZZZZZZZZZZ",
  "ZZ    ZZZZZZ      ZZZZZZZZZZZZZZZZ         ZZZZZZZZ   ZZZXXXXXXX",
  "ZZ  ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZX     X",
  "ZZ                                                             X",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZX     X",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZXXXXXXX",
};

static const char data_level2[H][W + 1] =
{
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
  "ZZ             ZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "ZZ                                 A                          ZZ",
  "ZZ  !          *                   A       *                  ZZ",
  "ZZZZZZ     ZZZZZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZ   ZZZZZZZZZZZZZZZZZZZZZZZZZZZZ      ZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZ   ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ     ZZ",
  "XXXXXXZZZZZZZZZZZZZZZZZZZ  ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ    ZZZ",
  "XXXXXX         ZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "Xa                       Z        ZZ                       ZZZZZ",
  "XZXXXX           *     Z   Z      ZZ                      ZZ  ZZ",
  "XXXXXX     ZZZZZZZZZZ            ZZZZZ           ZZZZ         ZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ   ZZZZZZZZZZZZZZZZZZZ",
  "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ   XXXXXZZZZZZZZZZZZZZ",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX         XXXXXXXXXXXXXXXXX",
  "XX                                        XXX   B       XXXXXXXX",
  "XX  b                           *         XXX   B           XXXX",
  "XX  Z  XXXXXXXXXXXXXXXXXX             XXXXXXXXXXX   *   XXX XXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX",
  "XXXXXXXXXXXXXXXXXXX                                         XXXX",
  "XXXXXXXXXXXXXXXXXXX                                         XXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXX       d    XXXX    d      XXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXDDDDDDDDDDDDXXXXDDDDDDDDDDDXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXX            XXXX           XXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXX            XXXX           XXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXX    *    *  XXXX   **      XXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
};

void loadLevel(Matrix<int>& tiles, Vector2i& start, IGame* game, int number)
{
  auto data = number ? data_level2 : data_level1;

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
      case 'X': tile = 2;
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

