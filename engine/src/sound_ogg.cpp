/*
 * Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "sound.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include "file.h"

using namespace std;

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
    if(!exists(filename))
      throw runtime_error("OggSound: file doesn't exist: '" + filename + "'");

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

