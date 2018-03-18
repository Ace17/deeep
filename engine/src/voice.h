// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// An audio voice

#pragma once

#include <memory>
#include "sound.h"

using namespace std;

auto const CHUNK_PERIOD = 32; // sample count between audio param updates

struct LoopingSource : IAudioSource
{
  LoopingSource(Sound* sound_) : sound(sound_) {}

  virtual int mix(Span<float> dst)
  {
    auto output = dst;

    while(output.len > 0)
    {
      if(!src)
        src = sound->createSource();

      auto const N = src->mix(output);
      output.len -= N;
      output.data += N;

      if(N == 0)
        src.reset();
    }

    return dst.len;
  }

private:
  Sound* sound;
  unique_ptr<IAudioSource> src;
};

struct Voice
{
  bool isDead() const
  {
    return m_isDead;
  }

  void play(Sound* sound, float fadeInInertia = 0, bool loop = false)
  {
    m_isDead = false;

    if(loop)
      m_player = make_unique<LoopingSource>(sound);
    else
      m_player = sound->createSource();

    m_alpha = fadeInInertia;
    m_volume = 0;
    m_targetVolume = 1;
  }

  void mix(Span<float> output)
  {
    startChunk();

    while(output.len > 0)
    {
      if(!m_player)
        break;

      float chunkData[CHUNK_PERIOD] = { 0 };
      auto chunk = makeSpan(chunkData);
      chunk.len = output.len;

      auto const N = m_player->mix(chunk);

      for(int i = 0; i < chunk.len; ++i)
        output.data[i] += chunk.data[i] * m_volume;

      output.len -= N;
      output.data += N;

      // finished playing?
      if(N == 0)
        m_player.reset();
    }

    if(!m_player)
      m_isDead = true;
  }

  void startChunk()
  {
    m_volume = m_alpha * m_volume + (1 - m_alpha) * m_targetVolume;
  }

private:
  float m_volume = 0.0;
  float m_alpha = 0.5;
  float m_targetVolume = 1.0;
  bool m_isDead = true;
  unique_ptr<IAudioSource> m_player;
};

