// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Playlist management, sound loading

#include "audio.h"
#include "audio_backend.h"
#include "sound.h"

#include "misc/file.h" // exists

#include <cmath> // sin
#include <cstdio> // printf
#include <cstring> // strcpy
#include <memory>
#include <vector>

using namespace std;

struct BleepSound : Sound
{
  unique_ptr<IAudioSource> createSource()
  {
    struct BleepSoundSource : IAudioSource
    {
      virtual int read(Span<float> output)
      {
        auto const MAX = 1000;
        auto const N = min(output.len, MAX - sampleCount);

        for(int i = 0; i < N; ++i)
        {
          output[i] = sin(440.0 * sampleCount * 3.1415 * 2.0) * 0.2;
          ++sampleCount;
        }

        return N;
      }

      int sampleCount = 0;
    };

    return make_unique<BleepSoundSource>();
  }
};

struct HighLevelAudio : Audio
{
  HighLevelAudio(unique_ptr<IAudioBackend> backend) : m_backend(move(backend))
  {
  }

  void loadSound(int id, const char* path) override
  {
    sounds.resize(max(id + 1, (int)sounds.size()));

    if(!File::exists(path))
    {
      printf("[audio] sound '%s' was not found, fallback on default sound\n", path);
      sounds[id] = make_unique<BleepSound>();
      return;
    }

    sounds[id] = loadSoundFile(path);
  }

  void playSound(int id) override
  {
    m_backend->playSound(sounds[id].get());
  }

  void playMusic(int id) override
  {
    char path[256];
    sprintf(path, "res/music/music-%02d.ogg", id);

    if(!File::exists(path))
    {
      printf("[audio] music '%s' was not found, fallback on default music\n", path);
      strcpy(path, "res/music/default.ogg");
      id = 0;
    }

    if(id == currMusic)
      return;

    currMusic = id;
    printf("[audio] playing music: %s\n", path);

    musicChannel = m_backend->playLoop(loadSoundFile(path).release());
  }

  void stopMusic() override
  {
    m_backend->stopLoop(musicChannel);
  }

  int currMusic = -1;
  int musicChannel = -1;
  const unique_ptr<IAudioBackend> m_backend;
  vector<unique_ptr<Sound>> sounds;
};

///////////////////////////////////////////////////////////////////////////////

IAudioBackend* createAudioBackend();

Audio* createAudio()
{
  return new HighLevelAudio(std::unique_ptr<IAudioBackend>(createAudioBackend()));
}

