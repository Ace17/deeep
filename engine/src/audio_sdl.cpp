// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// SDL audio output

#include "audio.h"
#include "sound.h"

#include <memory>
#include <vector>
#include <atomic>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <SDL.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include "file.h"
#include "voice.h"

#include "base/util.h"
#include "base/span.h"

using namespace std;

auto const MAX_VOICES = 16;

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
      throw runtime_error("Can't open audio");

    {
      int freq = audiospec.freq;
      int channels = audiospec.channels;
      printf("[audio] %d Hz %d channels\n", freq, channels);
    }

    voices.resize(MAX_VOICES);

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

    auto snd = loadSoundFile(path);

    sounds.resize(max(id + 1, (int)sounds.size()));

    sounds[id] = move(snd);
  }

  void playSound(int id) override
  {
    auto sound = sounds[id].get();

    auto voice = allocVoice();

    if(!voice)
      return;

    voice->play(sound);
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
    voices[0].fadeOut();
    m_nextMusic = move(nextMusic);
    SDL_UnlockAudioDevice(audioDevice);
  }

  static void staticMixAudio(void* userData, Uint8* stream, int iNumBytes)
  {
    auto pThis = (SdlAudio*)userData;
    memset(stream, 0, iNumBytes);
    pThis->mixAudio((float*)stream, iNumBytes / sizeof(float));
  }

  int currMusic = -1;
  vector<unique_ptr<Sound>> sounds;
  SDL_AudioDeviceID audioDevice;

  // accessed by the audio thread
  SDL_AudioSpec audiospec;
  vector<Voice> voices;
  unique_ptr<Sound> m_music;
  unique_ptr<Sound> m_nextMusic;
  vector<float> mixBuffer;

  void mixAudio(float* stream, int sampleCount)
  {
    if(m_nextMusic && voices[0].isDead())
    {
      m_music = move(m_nextMusic);
      voices[0].play(m_music.get(), 4, true);
    }

    int shift = 0;

    // HACK: poor man's resampling
    if(audiospec.freq > 22050)
      shift = 1;

    for(auto& val : mixBuffer)
      val = 0;

    Span<float> buff {
      mixBuffer.data(), sampleCount >> shift
    };

    while(buff.len > 0)
    {
      auto chunk = buff;
      chunk.len = min(CHUNK_PERIOD, chunk.len);

      for(auto& voice : voices)
        if(!voice.isDead())
          voice.mix(chunk);

      buff.data += chunk.len;
      buff.len -= chunk.len;
    }

    for(int i = 0; i < sampleCount; ++i)
      stream[i] = mixBuffer[i >> shift];
  }

  Voice* allocVoice()
  {
    for(int k = 1; k < MAX_VOICES; ++k)
      if(voices[k].isDead())
        return &voices[k];

    return nullptr;
  }
};

Audio* createAudio()
{
  return new SdlAudio;
}

