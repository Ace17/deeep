// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// low-level utilities, completing the C++ standard library

#pragma once

#include <memory>
#include <vector>

using namespace std;

template<typename T>
unique_ptr<T> unique(T* p)
{
  return unique_ptr<T>(p);
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

template<typename Container, typename Element>
bool exists(Container const& c, Element const& e)
{
  return c.find(e) != c.end();
}

inline auto allPairs(int n)
{
  struct Range
  {
    int n;

    struct State
    {
      int n;
      int i;
      int j;

      pair<int, int> operator * () const
      {
        return pair<int, int>(i, j);
      }

      bool operator != (State const& other) const
      {
        return n != other.n || i != other.i || j != other.j;
      }

      void operator ++ ()
      {
        ++j;

        if(j >= n)
        {
          ++i;
          j = i + 1;
        }

        if(i >= n || j >= n)
        {
          n = -1;
          i = -1;
          j = -1;
        }
      }
    };

    auto end()
    {
      return State({ -1, -1, -1 });
    }

    auto begin()
    {
      if(n <= 1)
        return end();

      return State({ n, 0, 1 });
    }
  };

  return Range({ n });
}

inline auto rasterScan(int cx, int cy)
{
  struct Iterable
  {
    int cx;
    int last;

    struct State
    {
      int cx;
      int i;

      pair<int, int> operator * () const
      {
        return pair<int, int>(i % cx, i / cx);
      }

      bool operator != (State const& other) const
      {
        return i != other.i;
      }

      void operator ++ ()
      {
        ++i;
      }
    };

    auto end()
    {
      return State({ cx, last });
    }

    auto begin()
    {
      return State({ cx, 0 });
    }
  };

  return Iterable({ cx, cx * cy });
}

// Remove an element from a vector. Might change the ordering.
template<typename T, typename Lambda>
void unstableRemove(vector<T>& container, Lambda predicate)
{
  for(int i = 0; i < (int)container.size(); ++i)
  {
    if(predicate(container[i]))
    {
      auto const j = (int)container.size() - 1;

      swap(container[i], container[j]);

      if(i != j)
        --i;

      container.pop_back();
    }
  }
}

