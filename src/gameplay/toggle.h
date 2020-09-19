/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

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

