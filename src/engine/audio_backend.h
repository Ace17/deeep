// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

const int SAMPLERATE = 22050;

// An audio backend doesn't receive messages.
// It continuously pulls from an IAudioMixer.
struct IAudioBackend
{
  virtual ~IAudioBackend() = default;
};

struct IAudioMixer;

IAudioBackend* createAudioBackend(IAudioMixer* mixer);

