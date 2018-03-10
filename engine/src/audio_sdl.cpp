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

#include "base/util.h"
#include "base/span.h"

using namespace std;

auto const MAX_VOICES = 16;

struct Voice
{
  bool isDead() const
  {
    return m_isDead;
  }

  void play(Sound* sound, bool loop = false)
  {
    m_sound = sound;
    m_player = sound->createPlayer();
    m_isDead = false;
    m_loop = loop;
  }

  void mix(Span<float> output)
  {
    while(output.len > 0)
    {
      auto const n = m_player->mix(output);
      output.data += n;
      output.len -= n;

      if(output.len == 0)
        break;

      m_sound = nextSound();

      if(!m_sound)
      {
        m_isDead = true;
        break;
      }

      m_player = m_sound->createPlayer();
    }
  }

private:
  Sound* nextSound()
  {
    if(m_loop)
      return m_sound;

    return nullptr;
  }

  bool m_isDead = true;
  bool m_loop = false;
  Sound* m_sound;
  unique_ptr<ISoundPlayer> m_player;
};

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
      printf("Audio: %d Hz %d channels\n", freq, channels);
    }

    voices.resize(MAX_VOICES);

    mixBuffer.resize(audiospec.samples * audiospec.channels);

    SDL_PauseAudioDevice(audioDevice, 0);
  }

  ~SdlAudio()
  {
    SDL_PauseAudioDevice(audioDevice, 1);

    SDL_CloseAudioDevice(audioDevice);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }

  void loadSound(int id, string path) override
  {
    if(!exists(path))
    {
      printf("sound '%s' was not found, fallback on default sound\n", path.c_str());
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
    if(id == currMusic)
      return;

    char path[256];
    sprintf(path, "res/music/music-%02d.ogg", id);

    if(!exists(path))
    {
      printf("music '%s' was not found, fallback on default music\n", path);
      strcpy(path, "res/music/default.ogg");
    }

    music = loadSoundFile(path);

    SDL_LockAudioDevice(audioDevice);
    voices[0].play(music.get(), true);
    currMusic = id;
    SDL_UnlockAudioDevice(audioDevice);
  }

  static void staticMixAudio(void* userData, Uint8* stream, int iNumBytes)
  {
    auto pThis = (SdlAudio*)userData;
    memset(stream, 0, iNumBytes);
    pThis->mixAudio((float*)stream, iNumBytes / sizeof(float));
  }

  SDL_AudioDeviceID audioDevice;
  SDL_AudioSpec audiospec;
  vector<Voice> voices;
  vector<unique_ptr<Sound>> sounds;
  unique_ptr<Sound> music;
  vector<float> mixBuffer;
  int currMusic = -1;

  void mixAudio(float* stream, int sampleCount)
  {
    int ratio = 1;

    // HACK: poor man's resampling
    if(audiospec.freq > 22050)
      ratio = 2;

    for(auto& val : mixBuffer)
      val = 0;

    Span<float> buff {
      mixBuffer.data(), sampleCount / ratio
    };

    for(auto& voice : voices)
      if(!voice.isDead())
        voice.mix(buff);

    for(int i = 0; i < sampleCount; ++i)
      stream[i] = mixBuffer[i / ratio];
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

