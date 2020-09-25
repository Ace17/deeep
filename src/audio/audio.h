// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

struct Audio
{
  virtual ~Audio() = default;

  virtual void loadSound(int id, const char* path) = 0;
  virtual void playSound(int id) = 0;
  virtual void playMusic(int id) = 0;
  virtual void stopMusic() = 0;
};

