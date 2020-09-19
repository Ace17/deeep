// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// SDL audio output

#include "audio_backend.h"
#include "audio_channel.h"
#include "sound.h"

#include <memory>
#include <stdexcept>
#include <SDL.h>
#include <vector>

#include "base/span.h"
#include "base/util.h"

using namespace std;

namespace
{
auto const MAX_CHANNELS = 16;
auto const LOOP_CHANNEL = 0;

struct SdlAudioBackend : IAudioBackend
{
  SdlAudioBackend()
  {
    auto ret = SDL_InitSubSystem(SDL_INIT_AUDIO);

    if(ret == -1)
      throw runtime_error("Can't init audio subsystem");

    SDL_AudioSpec desired {};
    desired.freq = 22050;
    desired.format = AUDIO_F32SYS;
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = &staticMixAudio;
    desired.userdata = this;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audiospec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if(audioDevice == 0)
    {
      printf("[audio] %s\n", SDL_GetError());
      throw runtime_error("Can't open audio");
    }

    printf("[audio] %d Hz %d channels\n",
           audiospec.freq,
           audiospec.channels);

    m_channels.resize(MAX_CHANNELS);

    mixBuffer.resize(audiospec.samples * audiospec.channels);

    SDL_PauseAudioDevice(audioDevice, 0);
    printf("[audio] init OK\n");
  }

  ~SdlAudioBackend()
  {
    SDL_PauseAudioDevice(audioDevice, 1);

    SDL_CloseAudioDevice(audioDevice);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    printf("[audio] shutdown OK\n");
  }

  void playSound(Sound* sound) override
  {
    SDL_LockAudioDevice(audioDevice);
    auto channel = allocChannel();
    SDL_UnlockAudioDevice(audioDevice);

    if(!channel)
    {
      printf("[audio] no channel available\n");
      return;
    }

    channel->play(sound);
  }

  int playLoop(Sound* sound) override
  {
    SDL_LockAudioDevice(audioDevice);

    if(!m_channels[LOOP_CHANNEL].isDead())
      m_channels[LOOP_CHANNEL].fadeOut();

    m_nextMusic.reset(sound);
    SDL_UnlockAudioDevice(audioDevice);

    return LOOP_CHANNEL;
  }

  void stopLoop(int channel) override
  {
    assert(channel == LOOP_CHANNEL);
    SDL_LockAudioDevice(audioDevice);
    m_channels[channel].fadeOut();
    SDL_UnlockAudioDevice(audioDevice);
  }

  SDL_AudioDeviceID audioDevice;

  // accessed by the audio thread
  SDL_AudioSpec audiospec;
  vector<AudioChannel> m_channels;
  unique_ptr<Sound> m_music;
  unique_ptr<Sound> m_nextMusic;
  vector<float> mixBuffer;

  static void staticMixAudio(void* userData, Uint8* stream, int iNumBytes)
  {
    auto pThis = (SdlAudioBackend*)userData;
    memset(stream, 0, iNumBytes);
    pThis->mixAudio((float*)stream, iNumBytes / sizeof(float));
  }

  void mixAudio(float* stream, int sampleCount)
  {
    if(m_nextMusic && m_channels[LOOP_CHANNEL].isDead())
    {
      m_music = move(m_nextMusic);
      m_channels[LOOP_CHANNEL].play(m_music.get(), 2, true);
    }

    int shift = 0;

    // HACK: poor man's resampling
    if(audiospec.freq > 22050)
      shift = 1;

    for(auto& val : mixBuffer)
      val = 0;

    Span<float> buff(
      mixBuffer.data(), sampleCount >> shift
      );

    while(buff.len > 0)
    {
      auto chunk = buff;
      chunk.len = min(CHUNK_PERIOD, chunk.len);

      for(auto& channel : m_channels)
        if(!channel.isDead())
          channel.mix(chunk);

      buff += chunk.len;
    }

    for(int i = 0; i < sampleCount; ++i)
      stream[i] = mixBuffer[i >> shift];
  }

  AudioChannel* allocChannel()
  {
    for(int k = 1; k < MAX_CHANNELS; ++k)
      if(m_channels[k].isDead())
        return &m_channels[k];

    return nullptr;
  }
};
}

IAudioBackend* createAudioBackend()
{
  return new SdlAudioBackend;
}

