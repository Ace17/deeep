// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// SDL audio output

#include "audio.h"
#include "sound.h"
#include "misc/file.h" // exists
#include "audio_channel.h"

#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <SDL.h>

#include "base/util.h"
#include "base/span.h"

using namespace std;

auto const MAX_CHANNELS = 16;

struct SdlAudio : Audio
{
  SdlAudio()
  {
    auto ret = SDL_InitSubSystem(SDL_INIT_AUDIO);

    if(ret == -1)
      throw runtime_error("Can't init audio subsystem");

    SDL_AudioSpec desired {};
    desired.freq = 22050;
    desired.format = AUDIO_F32SYS;
    desired.channels = 2;
    desired.samples = 4096; // workaround slowness on chrome causing dropouts
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

  ~SdlAudio()
  {
    SDL_PauseAudioDevice(audioDevice, 1);

    SDL_CloseAudioDevice(audioDevice);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    printf("[audio] shutdown OK\n");
  }

  void loadSound(int id, string path) override
  {
    if(!exists(path))
    {
      printf("[audio] sound '%s' was not found, fallback on default sound\n", path.c_str());
      path = "res/sounds/default.ogg";
    }

    sounds.resize(max(id + 1, (int)sounds.size()));
    sounds[id] = loadSoundFile(path);
  }

  void playSound(int id) override
  {
    auto channel = allocChannel();

    if(!channel)
    {
      printf("[audio] no channel available\n");
      return;
    }

    channel->play(sounds[id].get());
  }

  void playMusic(int id) override
  {
    char path[256];
    sprintf(path, "res/music/music-%02d.ogg", id);

    if(!exists(path))
    {
      printf("music '%s' was not found, fallback on default music\n", path);
      strcpy(path, "res/music/default.ogg");
      id = 0;
    }

    if(id == currMusic)
      return;

    currMusic = id;

    auto nextMusic = loadSoundFile(path);

    SDL_LockAudioDevice(audioDevice);

    if(!m_channels[0].isDead())
      m_channels[0].fadeOut();

    m_nextMusic = move(nextMusic);
    SDL_UnlockAudioDevice(audioDevice);
  }

  void stopMusic() override
  {
    SDL_LockAudioDevice(audioDevice);
    m_channels[0].fadeOut();
    SDL_UnlockAudioDevice(audioDevice);
  }

  int currMusic = -1;
  vector<unique_ptr<Sound>> sounds;
  SDL_AudioDeviceID audioDevice;

  // accessed by the audio thread
  SDL_AudioSpec audiospec;
  vector<AudioChannel> m_channels;
  unique_ptr<Sound> m_music;
  unique_ptr<Sound> m_nextMusic;
  vector<float> mixBuffer;

  static void staticMixAudio(void* userData, Uint8* stream, int iNumBytes)
  {
    auto pThis = (SdlAudio*)userData;
    memset(stream, 0, iNumBytes);
    pThis->mixAudio((float*)stream, iNumBytes / sizeof(float));
  }

  void mixAudio(float* stream, int sampleCount)
  {
    if(m_nextMusic && m_channels[0].isDead())
    {
      m_music = move(m_nextMusic);
      m_channels[0].play(m_music.get(), 2, true);
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

      buff.data += chunk.len;
      buff.len -= chunk.len;
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

Audio* createAudio()
{
  return new SdlAudio;
}

