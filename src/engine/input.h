// Copyright (C) 2020 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "delegate.h"

enum class Key
{
  Esc,
  F2,
  PrintScreen,
  Pause,
  Return,
  Left,
  Right,
  Up,
  Down,
  ScrollLock,
  Tab,
  CapsLock,
  C,
  N,
  R,
  X,
  Y,
  Z,
};

struct UserInput
{
  virtual ~UserInput() = default;

  virtual void process() = 0;

  virtual void listenToKey(Key key, Delegate<void(bool isDown)> func, bool modControl = false, bool modAlt = false) = 0;
  virtual void listenToQuit(Delegate<void()> onQuit) = 0;
  virtual void listenToMouseWheel(Delegate<void(int)> onWheel) = 0;
};

