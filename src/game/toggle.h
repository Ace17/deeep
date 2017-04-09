#pragma once

#include <algorithm>
using namespace std;

struct Toggle
{
  bool toggle(bool newState)
  {
    auto const oldState = state;
    state = newState;
    return newState && !oldState;
  }

  bool state = false;
};

template<typename T, typename U>
bool tryActivate(T& val, U delay)
{
  if(val > 0)
    return false;

  val = delay;
  return true;
}

template<typename T>
bool decrement(T& val)
{
  auto oldVal = val;

  if(val > 0)
    val--;

  return oldVal && !val;
}

