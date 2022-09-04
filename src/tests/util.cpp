// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/my_algorithm.h"
#include "base/util.h"
#include "misc/util.h"
#include "tests.h"
#include <vector>
using namespace std;

template<>
struct ToStringImpl<std::pair<int, int>>
{
  static std::string call(const std::pair<int, int>& val)
  {
    return "(" + std::to_string(val.first) + ", " + std::to_string(val.second) + ")";
  }
};

template<>
struct ToStringImpl<std::vector<std::pair<int, int>>>
{
  static std::string call(const std::vector<std::pair<int, int>>& val)
  {
    std::string r;

    bool first = true;
    r += "[";

    for(auto& element : val)
    {
      if(!first)
        r += ", ";

      r += testValueToString(element);
      first = false;
    }

    r += "]";

    return r;
  }
};

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
  assertEquals(std::string("test.obj"), setExtension("test.source", "obj"));
}

static bool operator == (const String& a, const String& b)
{
  if(a.len != b.len)
    return false;

  return memcmp(a.data, b.data, a.len) == 0;
}

template<>
struct ToStringImpl<String>
{
  static std::string call(const String& val)
  {
    return std::string(val.data, val.len);
  }
};

unittest("Util: dirName")
{
  assertEquals(std::string("Hello/World"), dirName("Hello/World/Goodbye.txt"));
}

unittest("Util: baseName")
{
  assertEquals(std::string("Goodbye.txt"), baseName("Hello/World/Goodbye.txt"));
}

unittest("Base: sort: simple")
{
  std::vector<int> array = { 1, 2, 3, 4, 6, 5 };
  std::vector<int> expected = { 1, 2, 3, 4, 5, 6 };

  my::sort(Span<int>(array), [] (int a, int b) { return a < b; });
  assertEquals(expected, array);
}

unittest("Base: sort: reversed")
{
  std::vector<int> array = { 60, 50, 40, 30, 20, 10 };
  std::vector<int> expected = { 10, 20, 30, 40, 50, 60 };

  my::sort(Span<int>(array), [] (int a, int b) { return a < b; });
  assertEquals(expected, array);
}

unittest("Base: sort: random")
{
  std::vector<float> array = { 63.3, 9.3, -5.13, 3.14, 55.55 };
  std::vector<float> expected = { 63.3, 55.55, 9.3, 3.14, -5.13 };

  my::sort(Span<float>(array), [] (float a, float b) { return a > b; });
  assertEquals(expected, array);
}

