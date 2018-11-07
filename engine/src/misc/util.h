#pragma once

#include <string>

using namespace std;

inline bool endsWith(string const& value, string const& ending)
{
  if(ending.size() > value.size())
    return false;

  return equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline string setExtension(string name, string ext)
{
  auto e = name.rfind('.');
  return name.substr(0, e) + "." + ext;
}

inline string dirName(string path)
{
  auto n = path.rfind('/');

  if(n == path.npos)
    return ".";

  return path.substr(0, n);
}

