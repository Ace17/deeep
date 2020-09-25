// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

struct Sound;

struct IAudioBackend
{
  virtual ~IAudioBackend() = default;

  virtual void playSound(Sound* sound) = 0;

  // takes ownership of 'sound'!
  virtual int playLoop(Sound* sound) = 0;
  virtual void stopLoop(int channel) = 0;
};

