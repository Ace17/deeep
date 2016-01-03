/**
 * @brief low-level utilities, completing the C++ standard library
 * @author Sebastien Alaiwan
 * @date 2015-11-10
 */

/*
 * Copyright (C) 2016 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <random>

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

inline string dirName(string path)
{
  auto n = path.rfind('/');

  if(n == path.npos)
    return ".";

  return path.substr(0, n);
}

template<typename T>
T clamp(T val, T min, T max)
{
  return val < min ? min : (val > max ? max : val);
}

template<typename T>
T abs(T val)
{
  return val < 0 ? -val : val;
}

template<typename T>
T ceilDiv(T number, T divisor)
{
  return (number + divisor - 1) / divisor;
}

template<typename Container, typename Element>
bool exists(Container const& c, Element const& e)
{
  return c.find(e) != c.end();
}

template<typename T>
struct reverse_adapter
{
  reverse_adapter(T& c) : c(c)
  {
  }

  typename T::reverse_iterator begin()
  {
    return c.rbegin();
  }

  typename T::reverse_iterator end()
  {
    return c.rend();
  }

  T& c;
};

template<typename T>
reverse_adapter<T> retro(T& c)
{
  return reverse_adapter<T>(c);
}

template<typename T>
struct reverse_const_adapter
{
  reverse_const_adapter(T const& c) : c(c)
  {
  }

  typename T::const_reverse_iterator begin()
  {
    return c.rbegin();
  }

  typename T::const_reverse_iterator end()
  {
    return c.rend();
  }

  T const& c;
};

template<typename T>
reverse_const_adapter<T> retro(T const& c)
{
  return reverse_const_adapter<T>(c);
}

template<typename T, typename Gen>
auto shuffle_inplace(vector<T> &v, Gen & gen)
{
  auto dist = uniform_int_distribution<int>(0, v.size() - 1);

  for(size_t i = 0; i < v.size(); ++i)
  {
    auto a = dist(gen);
    auto b = dist(gen);
    swap(v[a], v[b]);
  }
}

template<typename T, typename Gen>
auto shuffle(vector<T> const & input, Gen & gen)
{
  vector<T> r = input;
  shuffle_inplace(r, gen);
  return r;
}

inline
vector<int> seq(int start, int end)
{
  vector<int> r;

  for(int i = start; i <= end; ++i)
    r.push_back(i);

  return r;
}

template<typename T>
vector<T> extract(vector<T> const& input, vector<int> indices)
{
  vector<T> r;

  for(auto i : indices)
    r.push_back(input[i]);

  return r;
}

