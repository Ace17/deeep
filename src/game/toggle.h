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

struct Debouncer
{
  bool tryActivate(int newDelay)
  {
    if(cooldown > 0)
      return false;

    cooldown = newDelay;
    return true;
  }

  void cool()
  {
    cooldown = max(cooldown - 1, 0);
  }

  Int cooldown;
};

template<typename T>
void decrement(T& val)
{
  if(val > 0)
    val--;
}

