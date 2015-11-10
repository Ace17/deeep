/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <vector>
#include <memory>
#include <string>

using namespace std;

template<typename T>
class uvector : public vector<unique_ptr<T>>
{
};

template<typename T>
class uptr : public unique_ptr<T>
{
public:
  uptr(T* p) : unique_ptr<T>(p)
  {
  }
};

template<typename T>
uptr<T> unique(T* p)
{
  return uptr<T>(p);
}

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

