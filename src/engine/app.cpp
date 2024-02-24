// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Main loop timing.
// No game-specific code should be here,
// and no platform-specific code should be here.

#include "app.h"

#include <memory>
#include <string>
#include <vector>

#include "base/audio.h"
#include "base/geom.h"
#include "base/logger.h"
#include "base/renderer.h"
#include "base/scene.h"
#include "misc/file.h"
#include "misc/time.h"

#include "audio_backend.h"
#include "graphics_backend.h"
#include "input.h"
#include "ratecounter.h"
#include "stats.h"
#include "video_capture.h"

auto const GAMEPLAY_HZ = 100;
auto const RESOLUTION = Vec2i(768, 768);
auto const CAPTURE_FRAME_PERIOD = 40;

IGraphicsBackend* createGraphicsBackend(Vec2i resolution);
IRenderer* createRenderer(IGraphicsBackend* backend);
MixableAudio* createAudio();
UserInput* createUserInput();

Gauge ggFps("FPS");
Gauge ggTps("TPS");
Gauge ggTicksPerFrame("Ticks/Frame");
Gauge ggTickDuration("Tick duration");

// Implemented by the game-specific part
Scene* createGame(IRenderer* renderer, Audio* audio, Span<const string> argv);
extern const String GAME_NAME;

class App : public IApp
{
public:
  App(Span<char*> args)
    : m_args({ args.data, args.data + args.len })
  {
    m_graphicsBackend.reset(createGraphicsBackend(RESOLUTION));
    m_renderer.reset(createRenderer(m_graphicsBackend.get()));
    m_audio.reset(createAudio());
    m_audioBackend.reset(createAudioBackend(m_audio.get()));
    m_input.reset(createUserInput());

    m_graphicsBackend->setCaption(GAME_NAME);

    m_scene.reset(createGame(m_renderer.get(), m_audio.get(), m_args));

    m_lastTime = GetSteadyClockMs();
    m_lastDisplayFrameTime = GetSteadyClockMs();

    registerUserInputActions();
  }

  bool tick() override
  {
    m_input->process();

    auto const now = (int)GetSteadyClockMs();

    if(m_fixedDisplayFramePeriod)
    {
      while(m_lastDisplayFrameTime + m_fixedDisplayFramePeriod < now)
      {
        m_lastDisplayFrameTime += m_fixedDisplayFramePeriod;
        tickOneDisplayFrame(m_lastDisplayFrameTime);
      }
    }
    else
    {
      m_lastDisplayFrameTime = now;
      tickOneDisplayFrame(now);
    }

    return m_running != AppState::Exit;
  }

private:
  void tickOneDisplayFrame(int now)
  {
    auto freq = GAMEPLAY_HZ;

    if(m_fastForward)
      freq *= 10;

    if(m_slowMotion)
      freq /= 10;

    int ticksPerFrame = 0;

    while(1)
    {
      const auto nextTime = m_lastTime + (1000 + m_lastRemainder) / freq;

      if(nextTime >= now)
        break;

      m_lastTime = nextTime;
      m_lastRemainder = (1000 + m_lastRemainder) % freq;

      if(!m_paused && m_running == AppState::Running)
      {
        tickGameplay();
        m_tps.tick(now);
        ggTps = m_tps.slope();
      }

      ++ticksPerFrame;
    }

    ggTicksPerFrame = ticksPerFrame;

    // draw the frame
    draw();

    m_fps.tick(now);
    ggFps = m_fps.slope();
  }

  void tickGameplay()
  {
    m_control.debug = m_debugMode;

    auto const t0 = GetSteadyClockMs();

    m_scene->tick(m_control);

    auto const t1 = GetSteadyClockMs();
    ggTickDuration = int(t1 - t0);
  }

  void registerUserInputActions()
  {
    // App keys
    m_input->listenToQuit([&] () { m_running = AppState::Exit; });

    m_input->listenToKey(Key::PrintScreen, [&] (bool isDown) { if(isDown) toggleVideoCapture(); }, true);
    m_input->listenToKey(Key::PrintScreen, [&] (bool isDown) { if(isDown) m_recorder.takeScreenshot(); }, false);
    m_input->listenToKey(Key::Return, [&] (bool isDown) { if(isDown) toggleFullScreen(); }, false, true);

    m_input->listenToKey(Key::Y, [&] (bool isDown) { if(isDown && m_running == AppState::ConfirmExit) m_running = AppState::Exit; });
    m_input->listenToKey(Key::N, [&] (bool isDown) { if(isDown && m_running == AppState::ConfirmExit) m_running = AppState::Running; });

    // Player keys
    m_input->listenToKey(Key::Esc, [&] (bool isDown) { if(isDown) onQuit(); });
    m_input->listenToKey(Key::Return, [&] (bool isDown) { m_control.start = isDown; });

    m_input->listenToKey(Key::Left, [&] (bool isDown) { m_control.left = isDown; });
    m_input->listenToKey(Key::Right, [&] (bool isDown) { m_control.right = isDown; });
    m_input->listenToKey(Key::Up, [&] (bool isDown) { m_control.up = isDown; });
    m_input->listenToKey(Key::Down, [&] (bool isDown) { m_control.down = isDown; });

    m_input->listenToKey(Key::Z, [&] (bool isDown) { m_control.fire = isDown; });
    m_input->listenToKey(Key::X, [&] (bool isDown) { m_control.jump = isDown; });
    m_input->listenToKey(Key::C, [&] (bool isDown) { m_control.dash = isDown; });

    m_input->listenToKey(Key::R, [&] (bool isDown) { m_control.restart = isDown; });

    // Debug keys
    m_input->listenToKey(Key::F2, [&] (bool isDown) { if(isDown) m_scene.reset(createGame(m_renderer.get(), m_audio.get(), m_args)); });
    m_input->listenToKey(Key::Tab, [&] (bool isDown) { if(isDown) m_slowMotion = !m_slowMotion; });
    m_input->listenToKey(Key::Backtick, [&] (bool isDown) { m_fastForward = isDown; });
    m_input->listenToKey(Key::ScrollLock, [&] (bool isDown) { if(isDown) toggleDebug(); });
    m_input->listenToKey(Key::Pause, [&] (bool isDown) { if(isDown){ togglePause(); } });
  }

  void draw()
  {
    m_renderer->beginDraw();

    m_scene->draw();

    if(m_running == AppState::ConfirmExit)
    {
      RenderSprite s {};
      s.pos = { -8, -8 };
      s.halfSize = { 16, 16 };
      s.modelId = 0;
      s.zOrder = 99;
      m_renderer->drawSprite(s);
      m_renderer->drawText({ Vec2f(0, 0.5), "QUIT? [Y/N]" });
    }
    else if(m_paused)
      m_renderer->drawText({ Vec2f(0, 0), "PAUSE" });
    else if(m_slowMotion)
      m_renderer->drawText({ Vec2f(0, 0), "SLOW-MOTION MODE" });

    if(m_control.debug)
    {
      for(int i = 0; i < getStatCount(); ++i)
      {
        char txt[256];
        auto stat = getStat(i);
        auto s = format(txt, "%s: %.2f", stat.name, stat.val);
        m_renderer->drawText({ Vec2f(0, 4 - i), s });
      }
    }

    m_renderer->endDraw();

    m_recorder.captureDisplayFrameIfNeeded(m_graphicsBackend.get(), RESOLUTION);
  }

  void onQuit()
  {
    if(m_running == AppState::ConfirmExit)
      m_running = AppState::Running;
    else
      m_running = AppState::ConfirmExit;
  }

  void toggleVideoCapture()
  {
    if(m_fullscreen)
    {
      logMsg("Can't capture video in fullscreen mode");
      return;
    }

    if(m_recorder.toggleVideoCapture())
    {
      m_fixedDisplayFramePeriod = CAPTURE_FRAME_PERIOD;
      logMsg("Capturing video at %d Hz...", 1000 / CAPTURE_FRAME_PERIOD);
    }
    else
    {
      m_fixedDisplayFramePeriod = 0;
    }
  }

  void toggleFullScreen()
  {
    if(m_fixedDisplayFramePeriod)
    {
      logMsg("Can't toggle full-screen during video capture");
      return;
    }

    m_fullscreen = !m_fullscreen;
    m_graphicsBackend->setFullscreen(m_fullscreen);
  }

  void toggleDebug()
  {
    m_debugMode = !m_debugMode;
  }

  void togglePause()
  {
    // playSound(0)
    auto voiceId = m_audio->createVoice();
    m_audio->playVoice(voiceId, 0);
    m_audio->releaseVoice(voiceId, true);

    m_paused = !m_paused;
  }

  enum class AppState
  {
    Exit = 0,
    Running = 1,
    ConfirmExit = 2,
  };

  AppState m_running = AppState::Running;
  int m_fixedDisplayFramePeriod = 0;

  VideoCapture m_recorder;

  bool m_debugMode = false;

  int m_lastTime;
  int m_lastRemainder = 0;

  int m_lastDisplayFrameTime;
  RateCounter m_fps;
  RateCounter m_tps;
  Control m_control {};
  vector<string> m_args;
  bool m_slowMotion = false;
  bool m_fastForward = false;
  bool m_fullscreen = false;
  bool m_paused = false;
  unique_ptr<MixableAudio> m_audio;
  unique_ptr<IAudioBackend> m_audioBackend;
  unique_ptr<IRenderer> m_renderer;
  unique_ptr<IGraphicsBackend> m_graphicsBackend;
  unique_ptr<UserInput> m_input;

  unique_ptr<Scene> m_scene;
};

///////////////////////////////////////////////////////////////////////////////

unique_ptr<IApp> createApp(Span<char*> args)
{
  return make_unique<App>(args);
}

