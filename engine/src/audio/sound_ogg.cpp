// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// OGG sound file format loading.
// Uses stb_vorbis.

#include "sound.h"

#include "misc/file.h" // read

#include "stb_vorbis.c"
#include <cassert>
#include <stdexcept>
#include <string.h> // memcpy

using namespace std;

struct OggSoundPlayer : IAudioSource
{
  OggSoundPlayer(Span<uint8_t> data) : m_data(data)
  {
    m_decoder = stb_vorbis_open_memory(m_data.data, m_data.len, nullptr, nullptr);
    assert(m_decoder);
  }

  ~OggSoundPlayer()
  {
    stb_vorbis_close(m_decoder);
  }

  int read(Span<float> output)
  {
    return stb_vorbis_get_samples_float_interleaved(m_decoder, 2, output.data, output.len) * 2;
  }

  stb_vorbis* m_decoder;
  const Span<uint8_t> m_data;
};

struct OggSound : Sound
{
  OggSound(string filename)
  {
    if(!File::exists(filename))
      throw runtime_error("OggSound: file doesn't exist: '" + filename + "'");

    m_data = File::read(filename);
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

