// Audio stuff

/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cassert>
#include <memory>
#include <vector>
#include <fstream>
#include <atomic>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <SDL.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include "sound.h"

#include "base/util.h"
#include "base/span.h"

using namespace std;

auto const MAX_VOICES = 16;

struct ISoundPlayer
{
  virtual ~ISoundPlayer() {};
  virtual int mix(Span<float> output) = 0;
};

struct Sound
{
  virtual ~Sound() {};
  virtual unique_ptr<ISoundPlayer> createPlayer() = 0;
};

struct OggSoundPlayer : ISoundPlayer
{
  OggSoundPlayer(string filename)
  {
    auto fp = fopen(filename.c_str(), "rb");

    if(!fp)
      throw runtime_error("ogg: can't open '" + filename + "'");

    auto stream = ov_open(fp, &m_ogg, nullptr, 0);

    if(stream < 0)
      throw runtime_error("ogg: can't parse '" + filename + "'");

    vorbis_info* info = ov_info(&m_ogg, -1);

    if(info->channels != 2)
      throw runtime_error("ogg: non-stereo files are not supported ('" + filename + "')");

    if(info->rate != 22050)
      throw runtime_error("ogg: sampling rates other than 22050 Hz are not supported ('" + filename + "')");
  }

  ~OggSoundPlayer()
  {
    ov_clear(&m_ogg);
  }

  int mix(Span<float> output)
  {
    int r = 0;

    short m_buff[256];

    int sampleCount = output.len;

    while(sampleCount > 0)
    {
      auto const N = min(sampleCount, 256);
      auto const read = ov_read(&m_ogg, (char*)m_buff, N * 2, 0, 2, 1, nullptr);

      if(read <= 0)
        break;

      auto const gotSamples = read / 2;

      r += gotSamples;
      sampleCount -= gotSamples;

      for(int i = 0; i < gotSamples; ++i)
        *output.data++ += (m_buff[i] / 32768.0);
    }

    return r;
  }

  OggVorbis_File m_ogg;
};

struct OggSound : Sound
{
  OggSound(string filename)
  {
    m_filename = filename;
  }

  unique_ptr<ISoundPlayer> createPlayer()
  {
    return make_unique<OggSoundPlayer>(m_filename);
  }

  string m_filename;
};

unique_ptr<Sound> loadSoundFile(string filename)
{
  return make_unique<OggSound>(filename);
}

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

  int mix(Span<float> output)
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

    return output.len;
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
  Sound* m_baseSound;
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
    desired.samples = 256;
    desired.callback = &staticMixAudio;
    desired.userdata = this;

    ret = SDL_OpenAudio(&desired, &audiospec);

    if(ret == -1)
      throw runtime_error("Can't open audio");

    {
      int freq = audiospec.freq;
      int channels = audiospec.channels;
      printf("Audio: %d Hz %d channels\n", freq, channels);
    }

    voices.resize(MAX_VOICES);

    mixBuffer.resize(audiospec.samples * audiospec.channels);

    SDL_PauseAudio(0);
  }

  ~SdlAudio()
  {
    SDL_PauseAudio(1);

    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }

  void loadSound(int id, string path) override
  {
    if(!ifstream(path).is_open())
    {
      printf("sound '%s' was not found, fallback on default sound\n", path.c_str());
      path = "res/sounds/default.ogg";
    }

    auto snd = loadSoundFile(path);

    if(!snd)
      throw runtime_error("Can't load sound: '" + path + "' : " + SDL_GetError());

    sounds.resize(max(id + 1, (int)sounds.size()));

    sounds[id] = move(snd);
  }

  void playSound(int id) override
  {
    auto sound = sounds[id].get();

    assert(sound);

    // SDL_PauseAudio(1);
    auto voice = allocVoice();
    // SDL_PauseAudio(0);

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

    if(!ifstream(path).is_open())
    {
      printf("music '%s' was not found, fallback on default music\n", path);
      strcpy(path, "res/music/default.ogg");
    }

    music = loadSoundFile(path);

    if(!music)
      throw runtime_error(string("Can't load music: ") + path);

    SDL_LockAudio();
    voices[0].play(music.get(), true);
    currMusic = id;
    SDL_UnlockAudio();
  }

  static void staticMixAudio(void* userData, Uint8* stream, int iNumBytes)
  {
    auto pThis = (SdlAudio*)userData;
    memset(stream, 0, iNumBytes);
    pThis->mixAudio((float*)stream, iNumBytes / sizeof(float));
  }

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

