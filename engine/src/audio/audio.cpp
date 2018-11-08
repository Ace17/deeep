// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Playlist management, sound loading

#include "audio.h"
#include "misc/file.h" // exists
#include "audio_backend.h"
#include "sound.h"

#include <cstdio> // printf
#include <cstring> // strcpy
#include <vector>
#include <memory>

using namespace std;

IAudioBackend* createAudioBackend();

struct HighLevelAudio : Audio
{
  HighLevelAudio(unique_ptr<IAudioBackend> backend) : m_backend(move(backend))
  {
  }

  void loadSound(int id, const char* path) override
  {
    if(!exists(path))
    {
      printf("[audio] sound '%s' was not found, fallback on default sound\n", path);
      path = "res/sounds/default.ogg";
    }

    sounds.resize(max(id + 1, (int)sounds.size()));
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

    if(!exists(path))
    {
      printf("music '%s' was not found, fallback on default music\n", path);
      strcpy(path, "res/music/default.ogg");
      id = 0;
    }

    if(id == currMusic)
      return;

    currMusic = id;

    m_backend->playMusic(path);
  }

  void stopMusic() override
  {
    m_backend->stopMusic();
  }

  int currMusic = -1;
  const unique_ptr<IAudioBackend> m_backend;
  vector<unique_ptr<Sound>> sounds;
};

Audio* createAudio()
{
  return new HighLevelAudio(std::unique_ptr<IAudioBackend>(createAudioBackend()));
}

