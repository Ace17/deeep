#pragma once

#include "base/string.h"

#include <cstring> // memcmp
#include <string>

inline bool endsWith(String value, String suffix)
{
  if(suffix.len > value.len)
    return false;

  return memcmp(value.data + value.len - suffix.len, suffix.data, suffix.len) == 0;
}

inline std::string setExtension(String name, String ext)
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

inline String removeExtension(String path)
{
  int i = (int)path.len - 1;

  while(i > 0 && path.data[i] != '.')
    --i;

  return String(path.data, i);
}

inline String baseName(String path)
{
  const char* s = path.data + path.len - 1;

  while(s - 1 > path.data && s[-1] != '/')
    --s;

  return String(s, path.data + path.len - s);
}

