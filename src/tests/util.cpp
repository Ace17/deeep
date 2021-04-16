// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/util.h"
#include "misc/util.h"
#include "tests.h"
#include <algorithm>
#include <vector>
using namespace std;

unittest("Util: allPairs(1)")
{
  auto expected = vector<pair<int, int>>({});

  vector<pair<int, int>> result;

  for(auto p : allPairs(1))
    result.push_back(p);

  assertEquals(expected, result);
}

unittest("Util: allPairs simple")
{
  auto expected = vector<pair<int, int>>(
  {
    make_pair(0, 1),
    make_pair(0, 2),
    make_pair(1, 2),
  });

  vector<pair<int, int>> result;

  for(auto p : allPairs(3))
    result.push_back(p);

  assertEquals(expected, result);
}

unittest("Util: rasterScan simple")
{
  auto expected = vector<pair<int, int>>(
  {
    make_pair(0, 0),
    make_pair(1, 0),
    make_pair(0, 1),
    make_pair(1, 1),
    make_pair(0, 2),
    make_pair(1, 2),
  });

  vector<pair<int, int>> result;

  for(auto p : rasterScan(2, 3))
    result.push_back(p);

  assertEquals(expected, result);
}

unittest("Util: endsWith")
{
  assertTrue(endsWith("Hello World", "World"));
  assertTrue(!endsWith("Hello World", "You"));
}

unittest("Util: setExtension")
{
  assertEquals("test.obj", setExtension("test.source", "obj"));
}

static bool operator == (const String& a, const String& b)
{
  if(a.len != b.len)
    return false;

  return memcmp(a.data, b.data, a.len) == 0;
}

static
std::ostream& operator << (std::ostream& o, const String& v)
{
  for(auto& c : v)
    o << c;

  return o;
}

unittest("Util: dirName")
{
  assertEquals("Hello/World", dirName("Hello/World/Goodbye.txt"));
}

