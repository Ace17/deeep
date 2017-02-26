/**
 * @brief Audio stuff
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <vector>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <SDL_mixer.h>
#include "sound.h"

using namespace std;

struct SdlAudio : Audio
{
  SdlAudio()
  {
    auto ret = Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 1024);

    if(ret == -1)
      throw runtime_error("Can't open audio");

    ret = Mix_AllocateChannels(16);

    if(ret == -1)
      throw runtime_error("Can't allocate channels");

    playMusic(1);
  }

  ~SdlAudio()
  {
    for(auto chunk : sounds)
      Mix_FreeChunk(chunk);
  }

  void loadSound(int id, string path) override
  {
    auto snd = Mix_LoadWAV(path.c_str());

    if(!snd)
      throw runtime_error("Can't load sound: '" + path + "' : " + SDL_GetError());

    sounds.resize(max(id + 1, (int)sounds.size()));

    sounds[id] = snd;
  }

  void playSound(int id) override
  {
    Mix_PlayChannel(-1, sounds[id], 0);
  }

  void playMusic(int id) override
  {
    if(id == currMusic)
      return;

    char path[256];
    sprintf(path, "res/music/music-%02d.ogg", id);

    if(!ifstream(path).is_open())
    {
      printf("music '%s' was not found, fallback on default music\n", path);
      strcpy(path, "res/music/music-00.ogg");
    }

    auto music = Mix_LoadMUS(path);

    if(!music)
      throw runtime_error(string("Can't load music: ") + path);

    Mix_FadeInMusic(music, -1, 2000);
    currMusic = id;
  }

  vector<Mix_Chunk*> sounds;
  int currMusic = -1;
};

struct DummyAudio : Audio
{
  void loadSound(int id, std::string path) override
  {
    printf("sound[%d]: '%s'\n", id, path.c_str());
  }

  void playSound(int id) override
  {
    printf("sound: #%d\n", id);
  }

  void playMusic(int id) override
  {
    if(id == currMusic)
      return;

    printf("music: #%d\n", id);
    currMusic = id;
  }

  int currMusic = -1;
};

Audio* createAudio(bool dummy)
{
  if(dummy)
    return new DummyAudio;

  return new SdlAudio;
}

