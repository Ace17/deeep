// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/string.h"

struct Audio
{
  using VoiceId = int;

  virtual ~Audio() = default;

  virtual void loadSound(int soundId, String path) = 0;

  virtual VoiceId createVoice() = 0;
  virtual void releaseVoice(VoiceId id, bool autonomous = false) = 0;

  virtual void playVoice(VoiceId id, int soundId, bool looped = false) = 0;
  virtual void stopVoice(VoiceId id) = 0;

  virtual void setVoiceVolume(VoiceId id, float vol) = 0;
};

// Called by audio backends
struct IAudioMixer
{
  virtual void mixAudio(Span<float> dst) = 0;
};

struct MixableAudio : Audio, IAudioMixer
{
};

