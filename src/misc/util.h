#pragma once

#include "base/string.h"

#include <string>
#include <string.h> // memcmp

using namespace std;

inline bool endsWith(String value, String suffix)
{
  if(suffix.len > value.len)
    return false;

  return memcmp(value.data + value.len - suffix.len, suffix.data, suffix.len) == 0;
}

inline string setExtension(String name, String ext)
{
  int i = (int)name.len - 1;

  while(i > 0 && name.data[i] != '.')
    --i;

  return std::string(name.data, i) + "." + std::string(ext.data, ext.len);
}

inline String dirName(String path)
{
  while(path.len > 0 && path.data[path.len - 1] != '/')
    --path.len;

  if(path.len && path.data[path.len - 1] == '/')
    path.len--;

  if(path.len == 0)
    return ".";

  return path;
}

