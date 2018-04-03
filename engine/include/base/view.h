// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Outside world, as seen by the game

#pragma once

#include "geom.h"

typedef int SOUND;
typedef int MUSIC;

// This interface should act as a message sink.
// It should provide no way to query anything about the outside world.
struct View
{
  virtual void textBox(char const* msg) = 0;
  virtual void playMusic(MUSIC id) = 0;
  virtual void playSound(SOUND id) = 0;
  virtual void setCameraPos(Vector2f pos) = 0;
  virtual void setAmbientLight(float amount) = 0;
};

