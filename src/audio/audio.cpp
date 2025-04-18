// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Playlist management, sound loading

#include "base/audio.h"
#include "base/error.h"
#include "base/logger.h"
#include "engine/audio_backend.h"
#include "misc/file.h" // exists
#include "misc/stats.h"
#include "sound.h"

#include <atomic>
#include <cassert>
#include <cmath> // sin
#include <memory>
#include <unordered_map>
#include <vector>

namespace
{
Gauge ggAudioVoices("Audio voices");

template<typename T>
struct Fifo
{
  Fifo(int maxCount = 1024) : data(maxCount) {}

  void push(const T& element)
  {
    const auto currPos = m_writePos.load();
    const auto nextPos = (currPos + 1) % (int)data.size();

    if(nextPos == m_readPos.load())
      return; // queue full

    data[currPos] = element;
    m_writePos.store(nextPos);
  }

  bool pop(T& element)
  {
    const auto currPos = m_readPos.load();
    const auto nextPos = (currPos + 1) % (int)data.size();

    if(currPos == m_writePos.load())
      return false; // nothing to pop

    element = std::move(data[currPos]);
    m_readPos.store(nextPos);
    return true;
  }

private:
  std::vector<T> data;

  std::atomic<int> m_readPos {};
  std::atomic<int> m_writePos {};
};

template<typename Container, typename Lambda>
void removeWhen(Container& container, Lambda predicate)
{
  for(auto i = container.begin(); i != container.end();)
  {
    if(predicate(i->second))
      i = container.erase(i);
    else
      ++i;
  }
}

struct BleepSound : Sound
{
  static constexpr auto baseFreq = 440.0;
  static constexpr auto maxSamples = SAMPLERATE / 80; // integer number of periods

  std::unique_ptr<IAudioSource> createSource()
  {
    struct BleepSoundSource : IAudioSource
    {
      virtual int read(Span<float> output)
      {
        auto const N = std::min(output.len / 2, maxSamples - sampleCount);

        for(int i = 0; i < N; ++i)
        {
          output[2 * i + 0] = mySin(0.0 + baseFreq * sampleCount / SAMPLERATE);
          output[2 * i + 1] = mySin(0.5 + baseFreq * sampleCount / SAMPLERATE);
          ++sampleCount;
        }

        return N * 2;
      }

      static double mySin(double t) { return sin(t * 3.141592653589793 * 2.0); }

      int sampleCount = 0;
    };

    return std::make_unique<BleepSoundSource>();
  }
};

struct HighLevelAudio : MixableAudio
{
  HighLevelAudio() : m_bleepSound(std::make_shared<BleepSound>())
  {
  }

  void loadSound(int id, String path) override
  {
    auto i = m_sounds.find(id);

    if(i != m_sounds.end())
      m_sounds.erase(i);

    // if the file doesn't exist, don't try to load it:
    // this would cause an exception, crashing the wasm-version of the program.
    if(!File::exists(path))
    {
      logMsg("[audio] sound '%.*s' was not found, fallback on default sound", path.len, path.data);
      return;
    }

    m_sounds.insert({ id, loadSoundFile(path) });
  }

  VoiceId createVoice() override
  {
    const auto id = m_nextVoiceId++;

    m_commandQueue.push({ Opcode::CreateVoice, id });

    return id;
  }

  void releaseVoice(VoiceId id, bool autonomous) override
  {
    uint8_t flags = autonomous ? 1 : 0;
    m_commandQueue.push({ Opcode::ReleaseVoice, id, {}, {}, flags });
  }

  void setVoiceVolume(VoiceId id, float vol) override
  {
    m_commandQueue.push({ Opcode::SetVoiceVolume, id, vol });
  }

  void playVoice(VoiceId id, int soundId, bool looped) override
  {
    auto i_sound = m_sounds.find(soundId);
    auto sound = i_sound != m_sounds.end() ? i_sound->second : m_bleepSound;
    const auto code = looped ? Opcode::PlayVoiceLooped : Opcode::PlayVoice;
    m_commandQueue.push({ code, id, {}, sound });
  }

  void stopVoice(VoiceId id) override
  {
    m_commandQueue.push({ Opcode::StopVoice, id });
  }

  // Main thread data
  int m_nextVoiceId = 1;

  // Shared read-only data (Resources)
  std::unordered_map<int, std::shared_ptr<Sound>> m_sounds;
  std::shared_ptr<BleepSound> m_bleepSound;

  /////////////////////////////////////////////////////////////////////////////
  // Communication between the main thread and the audio backend thread
  /////////////////////////////////////////////////////////////////////////////

  enum class Opcode
  {
    CreateVoice,
    ReleaseVoice,
    PlayVoice,
    PlayVoiceLooped,
    StopVoice,
    SetVoiceVolume,
  };

  struct Command
  {
    Opcode op;
    VoiceId id;
    float floatVal {};
    std::shared_ptr<Sound> sound {};
    uint8_t flags {};
  };

  Fifo<Command> m_commandQueue;

  /////////////////////////////////////////////////////////////////////////////
  // Audio backend thread
  /////////////////////////////////////////////////////////////////////////////

  struct Fader
  {
    Fader(float val) : value(val), target(val) {}
    float value;
    float target;
    float speed = 0.05;

    operator float () const { return value; }

    void update()
    {
      if(value < target)
        value = std::min(value + speed, target);
      else if(value > target)
        value = std::max(value - speed, target);
    }
  };

  struct Voice
  {
    Fader vol = Fader(1.0);

    float commandVolume = 1;
    bool released = false;
    bool loop = false;
    bool finished = false;
    std::shared_ptr<Sound> sound;
    std::unique_ptr<IAudioSource> source;
  };

  std::unordered_map<VoiceId, Voice> m_voices;

  void mixAudio(Span<float> dst) override
  {
    processCommands();

    float buffer[4096] {};
    assert(dst.len <= int(sizeof buffer));

    for(auto& voicePair : m_voices)
    {
      auto& voice = voicePair.second;

      if(voice.finished)
        continue;

      assert(voice.sound);

      Span<float> buf = buffer;
      buf.len = dst.len;

      mixVoice(voice, buf, dst);
    }

    removeDeadVoices();

    ggAudioVoices = m_voices.size();
  }

  void mixVoice(Voice& voice, Span<float> buf, Span<float> dst)
  {
    while(buf.len > 0)
    {
      if(!voice.source)
        voice.source = voice.sound->createSource();

      const int len = voice.source->read(buf);

      for(int i = 0; i < len / 2; ++i)
      {
        voice.vol.update();

        dst[2 * i + 0] += buf[2 * i + 0] * voice.vol;
        dst[2 * i + 1] += buf[2 * i + 1] * voice.vol;
      }

      buf += len;
      dst += len;

      if(buf.len > 0)
      {
        if(voice.loop)
        {
          voice.source.reset();
        }
        else
        {
          voice.finished = true;
          break;
        }
      }
    }
  }

  void removeDeadVoices()
  {
    static auto isDead = [] (Voice& voice)
      {
        return voice.released && voice.finished;
      };

    removeWhen(m_voices, isDead);
  }

  void processCommands()
  {
    Command cmd;

    while(m_commandQueue.pop(cmd))
    {
      switch(cmd.op)
      {
      case Opcode::CreateVoice:
        m_voices[cmd.id] = {};
        m_voices[cmd.id].vol.value = 1;
        m_voices[cmd.id].vol.target = 1;
        m_voices[cmd.id].vol.speed = 0.1;
        m_voices[cmd.id].finished = true;
        break;
      case Opcode::PlayVoiceLooped:
        m_voices[cmd.id].loop = true;
      // fallthrough
      case Opcode::PlayVoice:
        m_voices[cmd.id].vol.target = m_voices[cmd.id].commandVolume;
        m_voices[cmd.id].vol.speed = 0.001;
        m_voices[cmd.id].sound = cmd.sound;
        m_voices[cmd.id].source.reset();
        m_voices[cmd.id].finished = false;

        // silence ourselves if we're a redundant voice
        for(auto& pair : m_voices)
        {
          if(pair.first == cmd.id)
            continue;

          if(pair.second.sound == cmd.sound && !pair.second.source && !pair.second.finished)
          {
            m_voices[cmd.id].vol.target = 0;
            m_voices[cmd.id].vol.speed = 1;
            m_voices[cmd.id].finished = true;
            break;
          }
        }

        break;
      case Opcode::StopVoice:
        m_voices[cmd.id].vol.target = 0;
        m_voices[cmd.id].vol.speed = 0.001;
        break;
      case Opcode::ReleaseVoice:
        m_voices[cmd.id].released = true;

        if(m_voices[cmd.id].loop || !cmd.flags)
          m_voices[cmd.id].finished = true;

        break;
      case Opcode::SetVoiceVolume:
        m_voices[cmd.id].commandVolume = cmd.floatVal;
        m_voices[cmd.id].vol.target = cmd.floatVal;
        break;
      }
    }
  }
};
}

MixableAudio* createAudio()
{
  return new HighLevelAudio;
}

