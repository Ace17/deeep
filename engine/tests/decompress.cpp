// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "engine/src/misc/decompress.h"
#include "tests.h"
#include <vector>
using namespace std;

unittest("Decompress: simple")
{
  auto toString = [] (vector<uint8_t> s)
    {
      return string(s.begin(), s.end());
    };

  const uint8_t input[] =
  {
    0x78, 0x9C, 0xF3, 0x48, 0xCD, 0xC9, 0xC9, 0xD7,
    0x51, 0x28, 0xCF, 0x2F, 0xCA, 0x49, 0x01, 0x00,
    0x1B, 0xD4, 0x04, 0x69,
  };
  assertEquals("Hello, world",
               toString(decompress(input)));
}

unittest("Decompress: big")
{
  // "[AAA...AAAA]" (126 'A')
  const uint8_t input[] =
  {
    0x78, 0x01, 0x8B, 0x76, 0x1C, 0x50, 0x10, 0x0B,
    0x00, 0x3E, 0x54, 0x20, 0xB7,
  };

  auto const output = decompress(input);
  assertEquals(128u, output.size());
  assertEquals('[', output[0]);
  assertEquals('A', output[1]);
  assertEquals('A', output[80]);
  assertEquals('A', output[126]);
  assertEquals(']', output[127]);
}

