// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// SDL audio output

#include "base/error.h"
#include "base/logger.h"
#include "base/span.h"
#include "base/util.h"

#include "audio/sound.h"
#include "base/audio.h" // IAudioMixer
#include "engine/audio_backend.h"

#include <SDL.h>

namespace
{
struct SdlAudioBackend : IAudioBackend
{
  SdlAudioBackend(IAudioMixer* mixer) : m_mixer(mixer)
  {
    auto ret = SDL_InitSubSystem(SDL_INIT_AUDIO);

    if(ret == -1)
      throw Error("Can't init audio subsystem");

    SDL_AudioSpec desired {};
    desired.freq = SAMPLERATE;
    desired.format = AUDIO_F32SYS;
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = &staticMixAudio;
    desired.userdata = this;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audiospec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if(audioDevice == 0)
    {
      logMsg("[sdl_audio] %s", SDL_GetError());
      throw Error("Can't open audio");
    }

    logMsg("[sdl_audio] %d Hz %d channels",
           audiospec.freq,
           audiospec.channels);

    SDL_PauseAudioDevice(audioDevice, 0);
    logMsg("[sdl_audio] init OK");
  }

  ~SdlAudioBackend()
  {
    SDL_PauseAudioDevice(audioDevice, 1);

    SDL_CloseAudioDevice(audioDevice);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    logMsg("[sdl_audio] shutdown OK");
  }

  SDL_AudioDeviceID audioDevice;

  // accessed by the audio thread
  SDL_AudioSpec audiospec;
  IAudioMixer* const m_mixer;

  static void staticMixAudio(void* userData, Uint8* stream, int iNumBytes)
  {
    auto pThis = (SdlAudioBackend*)userData;
    memset(stream, 0, iNumBytes);
    Span<float> dst;
    dst.data = (float*)stream;
    dst.len = iNumBytes / sizeof(float);
    pThis->m_mixer->mixAudio(dst);
  }
};
}

IAudioBackend* createAudioBackend(IAudioMixer* mixer)
{
  return new SdlAudioBackend(mixer);
}

