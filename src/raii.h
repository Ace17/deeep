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

