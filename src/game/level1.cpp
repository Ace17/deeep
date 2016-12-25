/**
 * @brief Level 1
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2016 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cassert>
#include "base/geom.h"
#include "base/util.h"
#include "game/game.h"
#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/entities/teleporter.h"
#include "game/entities/bonus.h"

int interpretTile(Vector2i pos, Vector2i& start, IGame* game, int val)
{
  switch(val)
  {
  case ' ':
    return 0;
  case '!':
    start = pos;
    return 0;
  case '?':
    {
      auto teleporter = new Teleporter;
      teleporter->pos = Vector2f(pos.x, pos.y);
      game->spawn(teleporter);
      return 0;
    }
    return 0;
  case '@':
    {
      auto bonus = new Bonus;
      bonus->pos = Vector2f(pos.x, pos.y);
      game->spawn(bonus);
      return 0;
    }
  default:
    return 4;
  case '.':
    return 1;
  case 'X': return 5;
  case 'O': return 3;
  case 'Z': return 4;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
    {
      auto sw = new Switch(val - 'a');
      sw->pos = Vector2f(pos.x + 0.3, pos.y + 0.15);
      game->spawn(sw);
      return 0;
    }
  case 'A':
  case 'B':
  case 'C':
  case 'D':
    {
      auto sw = new Door(val - 'A', game);
      sw->pos = Vector2f(pos.x + 0.5, pos.y + 0.5);
      game->spawn(sw);
      return 0;
    }
  case '*':
    {
      auto wh = new Wheel;
      wh->pos = Vector2f(pos.x, pos.y);
      game->spawn(wh);
      return 0;
    }
  }
}

const auto W = 128;
const auto H = 128;

static const char data_level1[H][W + 1] =
{
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXX      XXYYYYYYYXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXEEEEEEEEXXXXXXXXXYYYYYYYEEEEEEEEXXXXXXXXEEEEEEEEXXXXXXXX",
  "XX             XXY                      X      X                                      XXXY                                    XX",
  "XX             XXY                                                                    XXXY                                    XX",
  "XX             XXY                                                                XX  XXXY                                XX  XX",
  "XX         d   XXY                        EEEE        XX           *              XX XXXXY                                XX XXX",
  "XX      X ZZZ  XXYYYYYYYXXXXXXXXXX    XXXXEEEEXXXX    XXXXXXXXXXXXXXXXXXEEEEEEEXXXXXXXXXXYYYYYYYEEEEEEEEXX    XXEEEEEEEEX XXXXXX",
  "XX      XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXX",
  "XX    XXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXX",
  "XX    XXXXXXXXXXXZZXXZZXXXXXXXXXXX    XXXXXXXXXXXX    XXXXXXXXXXEEEEEEEEXZZXXZZXXXXXXXXXXYYYYYYYXXXXXXXXX      XXZZXXZZXXXXXXXXX",
  "XX      X      X ZZ  ZZ         XX    XX      XXXX                       ZZ  ZZ       XXXY                       ZZ  ZZ       XX",
  "XX                                            XXXX                                    XXXY                 YY                 XX",
  "XX                                            XXXX                                    XXXY                 YY                 XX",
  "XX        EEEE   ZZ  ZZ                       XXXX                       ZZ  ZZ       XXXY                       ZZ  ZZ       XX",
  "XXXXXXXXXXEEEEXXXZZXXZZXXX    XXXXXXXXXXXXXXXXXXXX    XXXX    XXEEEEEEEEXZZXXZZXXX    XXXYYYYYYYXX    XXX      XXZZXXZZXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXX    XXXX    XXXXXXXXXXXXXXXXXXXXAAAAXXXXXXXXXXXX    XXXX    XXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXX      XXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXX      XXE    EEXXXXXXXXXXXXXXXXX",
  "X      XEEEEEEEEXXXXXXXXXX    XXXXXXXXXXXYYYYYYY     XXXXX    XXXYYYYYYYXXXXXXXXXX    XXXXXXXXXX     XXXE    EEXXZZXXZZXXXXXXXXX",
  "X                       XX    XX      XXXY          XXXXXX    XXXY                                  XXXXE    EEX ZZ  ZZ       XX",
  "X                                     XXXY         XXXXXXX    XXXY                                 XXXXXE                     XX",
  "X                                     XXXY        XXXXXXXX    XXXY                 ZZ             XXXXXXE                     XX",
  "X      XX               *             XXXY       XXXXXXXXX    XXXY                               XXXXXXXE    EEX ZZ  ZZ       XX",
  "X ZZZ  XXEEEEEEEXXXXXXXXXXXXXXXXXXXXXXXXXYYYYYYYXXXXXXXXXXYYYYXXXYYYYYYYXXXXXXXXXX    XXXXXXXXXXXXXXXXXXE    EEXXZZXXZZXXX    XX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXEEEEEEEXXXXXXXXXXX    XX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XX",
  "XXXXXXXX        EEEEEEEEXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXYYYYYYYXXXXXXXX     XXXXYYYYYYYXXXXXXXXXXXXXXXXXXXXXXXXXX    XX",
  "XX                      X      X                              XXXY                  XXXXXY              X      X              XX",
  "XX        ZZZZ                                                XXXY                 XXXXXXY                                    XX",
  "XX                                                            XXXY                XXXXXXXY                                    XX",
  "XX                        EEEE                                XXXY               XXXXXXXXY                EEEE                XX",
  "XX      ZZZZZZZZEEEEEEEEXXEEEEXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXYYYYYYYXX    XXXXXXXXXXXYYYYYYYXX    XXXXEEEEXXXX    XXXX    XX",
  "XX      XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXX    XX",
  "E      XXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXX    XX",
  "E     XXXYYYYYYY        XXXXXXXXXX    XXXXXXXXXXXXXXXXXXXZZXXZZX        XX    XXXXXXXXXXX      XXX    XXX      XXX    XXXX    XX",
  "E  XXXXXXY                    XXXX    XXXX     C         ZZ  ZZ         XX    XX      XXX       XX    XX       XXX    XXXX    XX",
  "E  X   XXY        ZZZZ        XXXX    XXXX     C                  ZZZZ                XXX                      XXX    XXXX    XX",
  "E  X  cXXY                    XXXX    XXXX     C                            *         XXX                      XXX    XXXX    XX",
  "E    XXXXY                    XXXX    XXXX     C         ZZ  ZZ                       XXX  b               a   XXX    XXXX    XX",
  "E    XXXXYYYYYYYZZZZZZZZ      XXXX    XXXX    XXXXXXXXXXXZZXXZZXZZZZZZZZXXXXXXXXXXXXXXXXX ZZZ  XXXXXXXXXX ZZZ  XXXYYYYXXXXYYYYXX",
  "EEEEEXXXXXXXXXXXXXXXXXXX      XXXX    XXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXX    XXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XYYYYYYYXXXXXXXXXXXXXXXXXX    XXXX    XXXX    XXXZZXXZZXXXXXXXXXXXXXXXXXXYYYYYYYEEEEEEEEEEEEEEEEEEEEEEEEXZZXXZZXXXXXXXXXXXXXXXXX",
  "XY      X      XX      XXX    XXXX    XXXX    XX ZZ  ZZ               XXXY                               ZZ  ZZ               XX",
  "XY                                                                    XXXY                 *                                  XX",
  "XY                                                                XX  XXXY                                                    XX",
  "XY        EEEE    EEEE                           ZZ  ZZ           XX XXXXY                               ZZ  ZZ               XX",
  "XYYYYYYYXXEEEEXXXXEEEEXXXXXXXXXXXXXXXXXXXXXXXXXXXZZXXZZXXX    XXX XXXXXXXYYYYYYYEEEEEEEEEEEEEEEEEEEEEEEEXZZXXZZXXX    XXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXX",
  "XYYYYYYYXXXXXXXXEEEEEEEEEEEEEEEEXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXYYYYYYYXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXX",
  "XY                                              X      XXX    XX      XXXY                    XXXX    XXXX                    XX",
  "XY                                                                    XXXY                    XXXX    XXXX         YY         XX",
  "XY                                                                XX  XXXY                    XXXX    XXXX         YY         XX",
  "XY                                                EEEE            XX XXXXY                    XXXX    XXXX                    XX",
  "XYYYYYYYXXXXXXXXEEEEEEEEEEEEEEEEXX    XXXXXXXXXXXXEEEEXXXXXXXXXXX XXXXXXXYYYYYYYXX    XXXXXXXXXXXX    XXXX    XX        XXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXX    XX        XXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXX    XXE    EEXXXXXXXXX",
  "XYYYYYYYXXXXXXXXEEEEEEEEXXXXXXXXXX    XXXZZXXZZXXXXXXXXXXXXXXXXX        XXXXXXXXXX    XXXXXXXXXXXX    XXXX    XXE    EEXXXXXXXXX",
  "XY                              XX    XX ZZ  ZZ X      XX      X              XXXX            XXXX            XXE    EEX      XX",
  "XY                                                                ZZZZ        XXXX            XXXX            XXE             XX",
  "XY                                                                        XX  XXXX            XXXX            XXE         XX  XX",
  "XY                                       ZZ  ZZ   EEEE    EEEE            XX XXXXX            XXXX            XXE    EEX  XX XXX",
  "XYYYYYYYXXXXXXXXEEEEEEEEXX    XXXXXXXXXXXZZXXZZXXXEEEEXXXXEEEEXXZZZZZZZZX XXXXXXXX    XXXXXXXXXXXX    XXXX    XXE    EEXX XXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXX    XXEEEEEEEXXXXXXXXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXX    XXXXXXXXXXXXXXXXXX",
  "XYYYYYYYXXXXXXXXXYYYYYYYXX    XXXXXXXXXXXXXXXXXXXZZXXZZXXXXXXXXXXXXXXXXX        XX    XX        XX    XXXX    XXXXXXXXXXXXXXXXXX",
  "XY            XXXY      XX    XX                 ZZ  ZZ                         XX    XX              XXXX      X      X      XX",
  "XY            XXXY                                                        EEEE            ZZZZ     YY XXXX                    XX",
  "XY            XXXY                                                        E                        YY XXXX                    XX",
  "XY            XXXY                               ZZ  ZZ            *      E            *              XXXX        EEEE        XX",
  "XYYYYYYYXX    XXXYYYYYYYXXXXXXXXXX    XXXXXXXXXXXZZXXZZXXXXXXXXXXXXXXXXXEEEEEEEEXXXXXXXXZZZZZZZZXXXXXXXXXX    XXXXEEEEXXXX    XX",
  "XXXXXXXXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XX",
  "XXXXXXXXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XX",
  "XXXXXXXXXX    XXXXXXXXXX        XX    XXXYYYYYYY        XXXXXXXXXZZXXZZX                XXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XX",
  "XX      XX    XX                      XXXY                       ZZ  ZZ                       XXXX    XXXX    XXXX            XX",
  "XX                        ZZZZ     YY XXXY        ZZZZ                    ZZZZ    ZZZZ        XXXX    XXXX    XXXX         YY XX",
  "XX                                 YY XXXY                                                    XXXX    XXXX    XXXX         YY XX",
  "XX                                    XXXY      *       XX       ZZ  ZZ                       XXXX    XXXX    XXXX            XX",
  "XX    XXXXXXXXXXXXXXXXXXZZZZZZZZXXXXXXXXXYYYYYYYZZZZZZZZXX    XXXZZXXZZXZZZZZZZZZZZZZZZZ      XXXX    XXXXYYYYXXXX      XXXXXXXX",
  "XX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXX      XXXX    XXXXXXXXXXXX      XXXXXXXX",
  "E    EEXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXE    EEXXXXXXXXXXX    XXXXXXXXXX",
  "E    EEXXXXXXXXXEEEEEEEEEEEEEEEEXXXXXXXXXYYYYYYYEEEEEEEE     XXXXYYYYYYY        XXXXXXXXXX    XXE    EEXXZZXXZZXXX    XXXXXXXXXX",
  "E    EEX                              XXXY                  XXXXXY                            XXE    EEX ZZ  ZZ               XX",
  "E                                     XXXY                 XXXXXXY        ZZZZ                XXE                             XX",
  "E                                     XXXY                XXXXXXXY                            XXE                  ZZ     XX  XX",
  "E    EEX                              XXXY               XXXXXXXXY                            XXE    EEX ZZ  ZZ           XX XXX",
  "E    EEXXX    XXEEEEEEEEEEEEEEEEXXXXXXXXXYYYYYYYEEEEEEEEXXXXXXXXXYYYYYYYZZZZZZZZXX    XXXX    XXE    EEXXZZXXZZXXX    XXX XXXXXX",
  "EEEEEEEXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXX    XXEEEEEEEXXXXXXXXXXX    XXXXXXXXXX",
  "XXXXXXXX        XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXX",
  "XYYYYYYY        EEEEEEEEEEEEEEEE        EEEEEEEEXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX     XXXXX    XXXXXXXXXXXXXXXXXXXX    XXXXXXXXXX",
  "XY               B                              X      XX      X                    XXXXXX                    XXXX            XX",
  "XY       YY  YY  B                ZZZZ      *                                      XXXXXXX                    XXXX      *     XX",
  "XY       YY  YY  B                                                                XXXXXXXX                    XXXX            XX",
  "XY               B                                EEEE    EEEE                   XXXXXXXXX                    XXXX            XX",
  "XYYYYYYY        EEEEEEEEEEEEEEEEZZZZZZZZEEEEEEEEXXEEEEXXXXEEEEXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXX        XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXX",
  "XXXXXXXXXX    XXXXXXXXXXXZZXXZZXXZZXXZZXEEEEEEEEXXXXXXXXXZZXXZZXXXXXXXXXXYYYYYYYXXXXXXXXXXXXXXXXXX    XXXXXXXXXXEEEEEEEEXXXXXXXX",
  "XX      XX    XX         ZZ  ZZ  ZZ  ZZ         X      X ZZ  ZZ       XXXY                    XXXX      X      X              XX",
  "XX                                                                    XXXY                    XXXX                            XX",
  "XX                                          *                         XXXY                    XXXX                            XX",
  "XX                       ZZ  ZZ  ZZ  ZZ           EEEE   ZZ  ZZ       XXXY                    XXXX        EEEE                XX",
  "XX    XXXXXXXXXXXX    XXXZZXXZZXXZZXXZZXEEEEEEEEXXEEEEXXXZZXXZZXXXXXXXXXXYYYYYYYXXXXXXXX      XXXX    XXXXEEEEXXEEEEEEEEXX    XX",
  "XX    XXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX      XXXX    XXXXXXXXXXXXXXXXXXXX    XX",
  "XX    XXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXDDDDXXXXXXXXXXXXXXXXXXXX    XX",
  "XX    XXXYYYYYYYXX    XXEEEEEEEEXXXXXXXXXZZXXZZXXXXXXXXXXYYYYYYYXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXX    XXXXXXXXXXXXXXXXXXXX    XX",
  "XX    XXXY      XX    XX        X      X ZZ  ZZ       XXXY      X      X                XX    XXXX    XXX      X      XXXX    XX",
  "XX    XXXY                                            XXXY                                                            XXXX    XX",
  "XX    XXXY                                            XXXY                                                            XXXX    XX",
  "XX    XXXY        *               EEEE   ZZ  ZZ       XXXY  *     EEEE                                    EEEE        XXXX    XX",
  "XX    XXXYYYYYYYXXXXXXXXEEEEEEEEXXEEEEXXXZZXXZZXXXXXXXXXXYYYYYYYXXEEEEXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXEEEEXXXX    XXXXYYYYXX",
  "XX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXX",
  "XX      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXX",
  "XXX             XXXXXXXXXXXXXXXXXXXXXXXX        XXXXXXXXXZZXXZZX        XXXXXXXXXX    XXXXXXXXXXXYYYYYYYXXXXXXXXXX    XXXXXXXXXX",
  "XXXX                          XXXX              X      X ZZ  ZZ         X      XXX    XX      XXXY              XX    XX      XX",
  "XXXXX     ZZZZ                XXXX        ZZZZ                    ZZZZ                        XXXY                            XX",
  "XXXXXX                    XX  XXXX                                                            XXXY                            XX",
  "XXXXXXX                   XX XXXXX                EEEE   ZZ  ZZ    *      EEEE                XXXY                            XX",
  "XXXXXXXXZZZZZZZZXX    XXX XXXXXXXX    XXZZZZZZZZXXEEEEXXXZZXXZZXZZZZZZZZXXEEEEXXXXXXXXXXXX    XXXYYYYYYYXX    XXXXXXXXXX      XX",
  "XXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXXXXXXXX      XX",
  "XXXXXXXXXXXXXXXX      XXXXXXXXXXXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX    XXXXXXXXXXXX    XXXXXXXXXXXX    XX",
  "XYYYYYYYXXXXXXXX     XXXXYYYYYYYXX    XXXXXXXXXXXXXXXXXXXXXXXXXXXZZXXZZXEEEEEE        XXXX    XXXYYYYYYYXX    XXXXXXXXXXXX    XX",
  "XY      X      X    XXXXXY      XX    XXX      X                 ZZ  ZZ          ZZZ  XXXX    XXXY      XX    XX      XXXX    XX",
  "XY                 XXXXXXY                                                       Z Z  XXXX    XXXY            *       XXXX    XX",
  "XY                XXXXXXXY                                                       ZZZ  XXXX    XXXY                    XXXX    XX",
  "XY  !     EEEE   XXXXXXXXY     *          EEEE      *            ZZ  ZZ               XXXX    XXXY                    XXXX    XX",
  "XYYYYYYYXXEEEEXXXXXXXXXXXYYYYYYYXXXXXXXXXXEEEEXXXXXXXXXXXXXXXXXXXZZXXZZXEEEEEEEEXXXXXXXXXXYYYYXXXYYYYYYYXXXXXXXXXXXXXXXXXXYYYYXX",
  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
};

void loadLevel1(Matrix<int>& tiles, Vector2i& start, IGame* game)
{
  auto data = data_level1;

  for(int y = 0; y < H; ++y)
    for(int x = 0; x < W; ++x)
    {
      auto val = data[H - 1 - y][x];

      auto pos = Vector2i(x, y);
      auto tile = interpretTile(pos, start, game, val);

      tiles.set(x, y, tile);
    }
}

