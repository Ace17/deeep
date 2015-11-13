#pragma once

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

