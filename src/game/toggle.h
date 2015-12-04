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

  Bool state;
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
void decrement(T& val)
{
  if(val > 0)
    val--;
}

