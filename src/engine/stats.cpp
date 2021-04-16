#include "stats.h"
#include <map>
#include <string>
#include <vector>

namespace
{
std::vector<StatVal> g_Values;
std::map<const char*, int> g_Map;
}

void Stat(const char* name, float value)
{
  auto i = g_Map.find(name);

  if(i == g_Map.end())
  {
    g_Map[name] = (int)g_Values.size();
    g_Values.push_back({ name, value });
    i = g_Map.find(name);
  }

  g_Values[i->second] = { name, value };
}

int getStatCount()
{
  return (int)g_Values.size();
}

StatVal getStat(int idx)
{
  return g_Values.at(idx);
}

