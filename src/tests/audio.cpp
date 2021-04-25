// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "engine/audio.h"
#include "tests.h"
#include <memory>
#include <vector>
using namespace std;

MixableAudio* createAudio();

unittest("Audio: sound loops without discontinuity")
{
  unique_ptr<MixableAudio> audio(createAudio());
  auto voice = audio->createVoice();
  audio->playVoice(voice, -1, true);

  float prevLeft = 0;
  float prevRight = 0;

  float maxDiffLeft = 0;
  float maxDiffRight = 0;

  for(int k = 0; k < 10; ++k)
  {
    float buffer[512] {};
    audio->mixAudio(buffer);

    for(int i = 0; i < 256; ++i)
    {
      float left = buffer[2 * i + 0];
      float right = buffer[2 * i + 1];

      maxDiffLeft = std::max<float>(maxDiffLeft, std::abs(left - prevLeft));
      maxDiffRight = std::max<float>(maxDiffRight, std::abs(right - prevRight));

      prevLeft = left;
      prevRight = right;
    }
  }

  // detect discontinuities
  assertTrue(maxDiffLeft < 0.07);
  assertTrue(maxDiffRight < 0.07);

  audio->releaseVoice(voice);
}

unittest("Audio: autonomous voice release")
{
  unique_ptr<MixableAudio> audio(createAudio());
  auto voice = audio->createVoice();
  audio->playVoice(voice, -1);
  audio->releaseVoice(voice, true);

  int soundDuration = 0;

  for(int k = 0; k < 20; ++k)
  {
    float buffer[128] {};
    audio->mixAudio(buffer);

    float rms = 0;

    for(auto val : buffer)
      rms += val * val;

    if(rms < 0.001)
      break;

    soundDuration += 128;
  }

  assertEquals(1280, soundDuration);
}

unittest("Audio: non-autonomous voice release: the sound gets stopped immediately")
{
  unique_ptr<MixableAudio> audio(createAudio());
  auto voice = audio->createVoice();
  audio->playVoice(voice, -1);
  audio->releaseVoice(voice, false);

  float buffer[128] {};
  audio->mixAudio(buffer);

  float rms = 0;

  for(auto val : buffer)
    rms += val * val;

  assertEquals(0, rms);
}

