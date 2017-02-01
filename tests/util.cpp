#include "base/util.h"
#include "tests/tests.h"
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

