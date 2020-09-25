// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "render/png.h"
#include "tests.h"
#include <vector>
using namespace std;

unittest("PNG: simple")
{
  unsigned char input[] =
  {
    0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x17,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xed, 0x34, 0xa4, 0xe7, 0x00, 0x00, 0x00,
    0x22, 0x49, 0x44, 0x41, 0x54, 0x38, 0xcb, 0xed, 0xcb, 0xb1, 0x0d, 0x00,
    0x00, 0x0c, 0x83, 0x30, 0xfe, 0x7f, 0x9a, 0x1e, 0x11, 0xa9, 0x13, 0xde,
    0x8d, 0x20, 0x0b, 0xc7, 0x9f, 0x24, 0x79, 0xa1, 0xdb, 0xd6, 0x03, 0xbc,
    0xcd, 0x09, 0xf7, 0x48, 0x09, 0x7b, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x49,
    0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
  };

  auto const bpp = 4;
  auto const W = 17;
  auto const H = 23;

  int width = 0, height = 0;
  auto pic = decodePng(input, width, height);
  assertEquals(W, width);
  assertEquals(H, height);

  // top-left is red
  assertEquals(0xFF, (int)pic[bpp * 0 + 0]);
  assertEquals(0x00, (int)pic[bpp * 0 + 1]);
  assertEquals(0x00, (int)pic[bpp * 0 + 2]);
  assertEquals(0xFF, (int)pic[bpp * 0 + 3]);

  // top-right is green
  assertEquals(0x00, (int)pic[bpp * (W - 1) + 0]);
  assertEquals(0xFF, (int)pic[bpp * (W - 1) + 1]);
  assertEquals(0x00, (int)pic[bpp * (W - 1) + 2]);
  assertEquals(0xFF, (int)pic[bpp * (W - 1) + 3]);

  // bottom-left is blue
  assertEquals(0x00, (int)pic[bpp * W * (H - 1) + 0]);
  assertEquals(0x00, (int)pic[bpp * W * (H - 1) + 1]);
  assertEquals(0xFF, (int)pic[bpp * W * (H - 1) + 2]);
  assertEquals(0xFF, (int)pic[bpp * W * (H - 1) + 3]);

  // bottom-right is white
  assertEquals(0xFF, (int)pic[bpp * (W * H - 1) + 0]);
  assertEquals(0xFF, (int)pic[bpp * (W * H - 1) + 1]);
  assertEquals(0xFF, (int)pic[bpp * (W * H - 1) + 2]);
  assertEquals(0xFF, (int)pic[bpp * (W * H - 1) + 3]);
}

