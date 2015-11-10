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

#include <stdexcept>
#include <algorithm>
#include <string>
#include <SDL_mixer.h>
#include "scene.h"

using namespace std;

static vector<Mix_Chunk*> sounds;

Mix_Chunk* loadSound(string path)
{
  auto snd = Mix_LoadWAV(path.c_str());

  if(!snd)
    throw runtime_error("Can't load sound: '" + path + "' : " + SDL_GetError());

  return snd;
}

void Audio_init()
{
  auto ret = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 512);

  if(ret == -1)
    throw runtime_error("Can't open audio");

  ret = Mix_AllocateChannels(16);

  if(ret == -1)
    throw runtime_error("Can't allocate channels");
}

void Audio_loadSound(int id, string path)
{
  sounds.resize(max(id + 1, (int)sounds.size()));
  sounds[id] = loadSound(path);
}

void Audio_destroy()
{
  for(auto chunk : sounds)
    Mix_FreeChunk(chunk);
}

void Audio_playSound(int id)
{
  Mix_PlayChannel(-1, sounds[id], 0);
}

