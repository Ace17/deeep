// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// OGG sound file format loading.
// Uses libogg/libvorbis.

#include "sound.h"
#include <string.h> // memcpy
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include "file.h" // read

using namespace std;

struct OggSoundPlayer : IAudioSource
{
  OggSoundPlayer(Span<uint8_t> data) : m_data(data)
  {
    ov_callbacks cbs =
    {
      &read_func,
      &seek_func,
      &close_func,
      &tell_func,
    };

    auto stream = ov_open_callbacks(this, &m_ogg, nullptr, 0, cbs);

    if(stream < 0)
      throw runtime_error("ogg: can't parse");

    vorbis_info* info = ov_info(&m_ogg, -1);

    if(info->channels != 2)
      throw runtime_error("ogg: non-stereo files are not supported");

    if(info->rate != 22050)
      throw runtime_error("ogg: only 22050 Hz sampling rate is supported");
  }

  ~OggSoundPlayer()
  {
    ov_clear(&m_ogg);
  }

  int read(Span<float> output)
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
        *output.data++ = (m_buff[i] / 32768.0);
    }

    return r;
  }

  OggVorbis_File m_ogg;
  const Span<uint8_t> m_data;
  int m_dataReadPointer = 0;

  static size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource)
  {
    auto pThis = (OggSoundPlayer*)datasource;
    auto const remainingSize = pThis->m_data.len - pThis->m_dataReadPointer;
    auto const totalSize = min<int>(remainingSize, size * nmemb);
    memcpy(ptr, pThis->m_data.data + pThis->m_dataReadPointer, totalSize);
    pThis->m_dataReadPointer += totalSize;
    return totalSize;
  }

  static int seek_func(void* datasource, ogg_int64_t offset, int whence)
  {
    auto pThis = (OggSoundPlayer*)datasource;
    switch(whence)
    {
    case SEEK_SET: pThis->m_dataReadPointer = (int)offset;
      break;
    case SEEK_CUR: pThis->m_dataReadPointer += (int)offset;
      break;
    default: return -1;
    }

    return 0;
  }

  static int close_func(void*)
  {
    return 0;
  }

  static long tell_func(void* datasource)
  {
    auto pThis = (OggSoundPlayer*)datasource;
    return pThis->m_dataReadPointer;
  }
};

struct OggSound : Sound
{
  OggSound(string filename)
  {
    if(!exists(filename))
      throw runtime_error("OggSound: file doesn't exist: '" + filename + "'");

    m_data = read(filename);
  }

  unique_ptr<IAudioSource> createSource()
  {
    auto data = Span<uint8_t> { (uint8_t*)m_data.data(), (int)m_data.size() };
    return make_unique<OggSoundPlayer>(data);
  }

  string m_data;
};

unique_ptr<Sound> loadSoundFile(string filename)
{
  return make_unique<OggSound>(filename);
}

