// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Main loop timing.
// No game-specific code should be here,
// and no platform-specific code should be here (SDL is OK).

#include <memory>
#include <string>
#include <vector>

#include "SDL.h"

#include "app.h"
#include "audio/audio.h"
#include "base/geom.h"
#include "base/resource.h"
#include "base/scene.h"
#include "base/view.h"
#include "input.h"
#include "misc/file.h"
#include "ratecounter.h"
#include "render/display.h"
#include "stats.h"

using namespace std;

auto const TIMESTEP = 10;
auto const RESOLUTION = Size2i(768, 768);

Display* createDisplay(Size2i resolution);
Audio* createAudio();
UserInput* createUserInput();

Scene* createGame(View* view, vector<string> argv);

class App : View, public IApp
{
public:
  App(Span<char*> args)
    : m_args({ args.data, args.data + args.len })
  {
    SDL_Init(0);

    m_display.reset(createDisplay(RESOLUTION));
    m_audio.reset(createAudio());
    m_input.reset(createUserInput());

    m_scene.reset(createGame(this, m_args));

    m_lastTime = SDL_GetTicks();
    m_lastDisplayFrameTime = SDL_GetTicks();

    registerUserInputActions();
  }

  virtual ~App()
  {
    SDL_Quit();
  }

  bool tick() override
  {
    m_input->process();

    auto const now = (int)SDL_GetTicks();

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

    return m_running;
  }

private:
  void tickOneDisplayFrame(int now)
  {
    auto const timeStep = m_slowMotion ? TIMESTEP * 10 : TIMESTEP;

    while(m_lastTime + timeStep < now)
    {
      m_lastTime += timeStep;

      if(!m_paused)
        tickGameplay();
    }

    {
      m_actors.clear();
      m_scene->draw();
      draw();
      m_fps.tick(now);
    }

    Stat("FPS", m_fps.slope());

    auto fps = m_fps.slope();
    if(fps != m_lastFps)
    {
      fpsChanged(fps);
      m_lastFps = fps;
    }

    captureDisplayFrameIfNeeded();
  }

  void captureDisplayFrameIfNeeded()
  {
    if(m_captureFile || m_mustScreenshot)
    {
      vector<uint8_t> pixels(RESOLUTION.width * RESOLUTION.height * 4);
      m_display->readPixels({ pixels.data(), (int)pixels.size() });

      if(m_captureFile)
        fwrite(pixels.data(), 1, pixels.size(), m_captureFile);

      if(m_mustScreenshot)
      {
        File::write("screenshot.rgba", pixels);
        fprintf(stderr, "Saved screenshot to 'screenshot.rgba'\n");

        m_mustScreenshot = false;
      }
    }
  }

  void tickGameplay()
  {
    auto next = m_scene->tick(m_control);

    if(next != m_scene.get())
    {
      m_scene.release();
      m_scene.reset(next);
      printf("Entering: %s\n", typeid(*next).name());
    }
  }

  void registerUserInputActions()
  {
    // App keys
    m_input->listenToKey(Key::PrintScreen, [&] (bool isDown) { if(isDown) toggleVideoCapture(); }, true);
    m_input->listenToKey(Key::Return, [&] (bool isDown) { if(isDown) toggleFullScreen(); }, false, true);
    m_input->listenToQuit([&] () { onQuit(); });

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
    m_input->listenToKey(Key::F2, [&] (bool isDown) { if(isDown) m_scene.reset(createGame(this, m_args)); });
    m_input->listenToKey(Key::Tab, [&] (bool isDown) { if(isDown) m_slowMotion = !m_slowMotion; });
    m_input->listenToKey(Key::ScrollLock, [&] (bool isDown) { if(isDown) m_control.debug = !m_control.debug; });
    m_input->listenToKey(Key::Pause, [&] (bool isDown) { if(isDown){ m_audio->playSound(0); m_paused = !m_paused; } });
  }

  void draw()
  {
    m_display->beginDraw();

    for(auto& actor : m_actors)
    {
      auto where = Rect2f(actor.pos.x, actor.pos.y, actor.scale.width, actor.scale.height);
      m_display->drawActor(where, actor.angle, !actor.screenRefFrame, (int)actor.model, actor.effect == Effect::Blinking, actor.action, actor.ratio, actor.zOrder);
    }

    if(m_paused)
      m_display->drawText(Vector2f(0, 0), "PAUSE");
    else if(m_slowMotion)
      m_display->drawText(Vector2f(0, 0), "SLOW-MOTION MODE");

    if(m_debugMode)
    {
      char debugText[256];
      sprintf(debugText, "FPS: %d", m_fps.slope());
      m_display->drawText(Vector2f(0, -4), debugText);
    }

    if(m_textboxDelay > 0)
    {
      auto y = 2.0f;
      auto const DELAY = 90.0f;

      if(m_textboxDelay < DELAY)
        y += 16 * (DELAY - m_textboxDelay) / DELAY;

      m_display->drawText(Vector2f(0, y), m_textbox.c_str());
      m_textboxDelay--;
    }

    m_display->endDraw();
  }

  void fpsChanged(int fps)
  {
    char title[128];
    sprintf(title, "%s (%d FPS)", m_title.c_str(), fps);
    m_display->setCaption(title);
  }

  void onQuit()
  {
    m_running = 0;
  }

  void toggleVideoCapture()
  {
    if(!m_captureFile)
    {
      if(m_fullscreen)
      {
        fprintf(stderr, "Can't capture video in fullscreen mode\n");
        return;
      }

      m_captureFile = fopen("capture.rgba", "wb");

      if(!m_captureFile)
      {
        fprintf(stderr, "Can't start video capture!\n");
        return;
      }

      m_fixedDisplayFramePeriod = 40;
      fprintf(stderr, "Capturing video at %d Hz...\n", 1000 / m_fixedDisplayFramePeriod);
    }
    else
    {
      fprintf(stderr, "Stopped video capture\n");
      fclose(m_captureFile);
      m_captureFile = nullptr;
      m_fixedDisplayFramePeriod = 0;
    }
  }

  void toggleFullScreen()
  {
    if(m_captureFile)
    {
      fprintf(stderr, "Can't toggle full-screen during video capture\n");
      return;
    }

    m_fullscreen = !m_fullscreen;
    m_display->setFullscreen(m_fullscreen);
  }

  // View implementation
  void setTitle(char const* gameTitle) override
  {
    m_title = gameTitle;
  }

  void preload(Resource res) override
  {
    switch(res.type)
    {
    case ResourceType::Sound:
      m_audio->loadSound(res.id, res.path);
      break;
    case ResourceType::Model:
      m_display->loadModel(res.id, res.path);
      break;
    }
  }

  void textBox(char const* msg) override
  {
    m_textbox = msg;
    m_textboxDelay = 60 * 2;
  }

  void playMusic(MUSIC id) override
  {
    m_audio->playMusic(id);
  }

  void stopMusic() override
  {
    m_audio->stopMusic();
  }

  void playSound(SOUND sound) override
  {
    m_audio->playSound(sound);
  }

  void setCameraPos(Vector2f pos) override
  {
    m_display->setCamera(pos);
  }

  void setAmbientLight(float amount) override
  {
    m_display->setAmbientLight(amount);
  }

  void sendActor(Actor const& actor) override
  {
    m_actors.push_back(actor);
  }

  int m_running = 1;
  int m_fixedDisplayFramePeriod = 0;
  FILE* m_captureFile = nullptr;
  bool m_mustScreenshot = false;

  bool m_debugMode = false;

  int m_lastTime;
  int m_lastDisplayFrameTime;
  int m_lastFps = -1;
  RateCounter m_fps;
  Control m_control {};
  vector<string> m_args;
  unique_ptr<Scene> m_scene;
  bool m_slowMotion = false;
  bool m_fullscreen = false;
  bool m_paused = false;
  unique_ptr<Audio> m_audio;
  unique_ptr<Display> m_display;
  vector<Actor> m_actors;
  unique_ptr<UserInput> m_input;

  string m_title;
  string m_textbox;
  int m_textboxDelay = 0;
};

///////////////////////////////////////////////////////////////////////////////

unique_ptr<IApp> createApp(Span<char*> args)
{
  return make_unique<App>(args);
}

