// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "engine/tests/tests.h"
#include "engine/src/voice.h"

struct DummyPlayer : IAudioSource
{
  DummyPlayer(int length) : m_length(length)
  {
  }

  virtual int read(Span<float> output)
  {
    const auto N = min(output.len, m_length - m_state);

    for(auto i = 0; i < N; ++i)
    {
      output.data[i] = m_state % 10 == 0 ? 1 : 2;
      m_state++;
    }

    return N;
  }

  int m_state = 0;
  const int m_length;
};

struct DummySound : Sound
{
  int length = 100;
  virtual std::unique_ptr<IAudioSource> createSource()
  {
    return make_unique<DummyPlayer>(length);
  }
};

template<size_t N>
bool buffEquals(float(&expected)[N], float(&actual)[N])
{
  bool mismatch = false;

  for(int i = 0; i < (int)N; ++i)
  {
    if(expected[i] != actual[i])
      mismatch = true;
  }

  if(mismatch)
  {
    fprintf(stderr, "Expected: ");

    for(auto& val : expected)
      fprintf(stderr, "%.0f ", val);

    fprintf(stderr, "\n");

    fprintf(stderr, "     Got: ");

    for(auto& val : actual)
      fprintf(stderr, "%.0f ", val);

    fprintf(stderr, "\n");

    return false;
  }

  return true;
}

unittest("Audio: play voice")
{
  DummySound sound;

  Voice v;
  v.play(&sound);

  float buffer[32] = { 0 };
  auto result = Span<float>(buffer);

  v.mix(result);

  float expected[32] = { 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2 };
  assert(buffEquals(expected, buffer));
}

unittest("Audio: play voice until end")
{
  DummySound sound;
  sound.length = 14;

  Voice v;
  v.play(&sound);

  float buffer[32] = { 0 };
  auto result = Span<float>(buffer);

  v.mix(result);

  float expected[32] =
  {
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2,
    // sound ends, then only silence
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  assert(buffEquals(expected, buffer));
}

unittest("Audio: play voice until end, loop")
{
  DummySound sound;
  sound.length = 14;

  Voice v;
  v.play(&sound, 0, true);

  float buffer[32] = { 0 };
  auto result = Span<float>(buffer);

  v.mix(result);

  float expected[32] =
  {
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2,
    // sound loops
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2,
    // sound loops
    1, 2, 2, 2
  };
  assert(buffEquals(expected, buffer));
}

