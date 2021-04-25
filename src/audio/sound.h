// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/span.h"
#include "base/string.h"
#include <memory>

// A sound being played (holds the current sound position)
struct IAudioSource
{
  virtual ~IAudioSource() = default;

  // returns the number of samples : unless EOS is reached,
  // the return value will be equal to output.len.
  virtual int read(Span<float> output) = 0;
};

// A chunk of audio
struct Sound
{
  virtual ~Sound() = default;
  virtual std::unique_ptr<IAudioSource> createSource() = 0;
};

std::unique_ptr<Sound> loadSoundFile(String filename);

