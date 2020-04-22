// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// An audio channel

#pragma once

#include "sound.h"
#include <cassert>
#include <memory>

using namespace std;

auto const CHUNK_PERIOD = 32; // sample count between audio param updates

struct LoopingSource : IAudioSource
{
  LoopingSource(Sound* sound_) : sound(sound_) {}

  virtual int read(Span<float> dst)
  {
    auto output = dst;

    while(output.len > 0)
    {
      if(!src)
        src = sound->createSource();

      auto const N = src->read(output);
      output += N;

      if(N == 0)
        src.reset();
    }

    return dst.len;
  }

private:
  Sound* sound;
  unique_ptr<IAudioSource> src;
};

struct AudioChannel
{
  bool isDead() const
  {
    return m_isDead;
  }

  void play(Sound* sound, float fadeInInertia = 0, bool loop = false)
  {
    m_isDead = false;

    if(loop)
      m_source = make_unique<LoopingSource>(sound);
    else
      m_source = sound->createSource();

    m_volume = 0;
    m_targetVolume = 1;
    m_volumeIncrement = (m_targetVolume - m_volume) / (200 * fadeInInertia);
  }

  void mix(Span<float> output)
  {
    assert(output.len <= CHUNK_PERIOD);
    startChunk();

    while(output.len > 0)
    {
      if(!m_source)
        break;

      float chunkData[CHUNK_PERIOD] = { 0 };
      auto chunk = Span<float>(chunkData);
      chunk.len = output.len;

      auto const N = m_source->read(chunk);

      for(int i = 0; i < chunk.len; ++i)
        output.data[i] += chunk.data[i] * m_volume;

      output += N;

      // finished playing?
      if(N == 0)
        m_source.reset();
    }

    if(!m_source)
      m_isDead = true;
  }

  void startChunk()
  {
    m_volume += m_volumeIncrement;

    if(m_volumeIncrement > 0)
    {
      if(m_volume > m_targetVolume)
        m_volume = m_targetVolume;
    }
    else
    {
      if(m_volume < m_targetVolume)
        m_volume = m_targetVolume;
    }

    if(m_fadingOut && m_volume == 0)
      m_isDead = true;
  }

  void fadeOut()
  {
    auto const fadeOutInertia = 4;
    m_fadingOut = true;
    m_targetVolume = 0.0;
    m_volumeIncrement = (m_targetVolume - m_volume) / (200 * fadeOutInertia);
  }

private:
  float m_volume = 0.0;
  float m_volumeIncrement = 0.0;
  float m_targetVolume = 1.0;
  bool m_fadingOut = false;
  bool m_isDead = true;
  unique_ptr<IAudioSource> m_source;
};

