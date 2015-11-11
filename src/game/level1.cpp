#include <cassert>
#include "engine/geom.h"
#include "engine/raii.h"
#include "game/game.h"
#include "game/entities/switch.h"

void loadLevel1(Matrix<int>& tiles, Vector2i& start, IGame* game)
{
  const auto W = 32 * 2;
  const auto H = 32 * 2;

  const char data[H][W + 1] =
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
          sw->pos = Vector2f(x, y);
          game->spawn(sw);
          break;
        }
      case 'A':
      case 'B':
      case 'C':
      case 'D':
        {
          auto sw = new Door(val - 'A', game);
          sw->pos = Vector2f(x, y);
          game->spawn(sw);
          break;
        }
      }

      tiles.set(x, y, tile);
    }
}

