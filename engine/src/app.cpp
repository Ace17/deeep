// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Main loop timing.
// No game-specific code should be here,
// and no platform-specific code should be here (SDL is OK).

#include <vector>
#include <string>
#include <memory>

#include "SDL.h"

#include "base/geom.h"
#include "base/resource.h"
#include "base/scene.h"
#include "base/view.h"
#include "ratecounter.h"
#include "audio.h"
#include "display.h"

using namespace std;

auto const TIMESTEP = 1;

Display* createDisplay(Size2i resolution);
Audio* createAudio();

Scene* createGame(View* view, vector<string> argv);

class App : View
{
public:
  App(vector<string> argv)
    : m_args(argv)
  {
    SDL_Init(0);

    m_display.reset(createDisplay(Size2i(768, 768)));
    m_audio.reset(createAudio());

    m_scene.reset(createGame(this, m_args));

    m_lastTime = SDL_GetTicks();
  }

  virtual ~App()
  {
    SDL_Quit();
  }

  bool tick()
  {
    processInput();

    auto const now = (int)SDL_GetTicks();
    bool dirty = false;

    while(m_lastTime < now)
    {
      m_lastTime += m_slowMotion ? TIMESTEP * 10 : TIMESTEP;

      if(!m_paused)
        m_scene->tick(m_control);

      dirty = true;
    }

    if(dirty)
    {
      draw();
      m_fps.tick(now);
    }

    auto fps = m_fps.slope();

    if(fps != m_lastFps)
    {
      fpsChanged(fps);
      m_lastFps = fps;
    }

    return m_running;
  }

private:
  void processInput()
  {
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
      case SDL_QUIT:
        onQuit();
        break;
      case SDL_KEYDOWN:
        onKeyDown(&event);
        break;
      case SDL_KEYUP:
        onKeyUp(&event);
        break;
      }
    }

    m_control.left = keys[SDL_SCANCODE_LEFT];
    m_control.right = keys[SDL_SCANCODE_RIGHT];
    m_control.up = keys[SDL_SCANCODE_UP];
    m_control.down = keys[SDL_SCANCODE_DOWN];

    m_control.fire = keys[SDL_SCANCODE_Z] || keys[SDL_SCANCODE_RETURN];
    m_control.jump = keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_SPACE];
    m_control.dash = keys[SDL_SCANCODE_C];

    m_control.restart = keys[SDL_SCANCODE_R];
    m_control.debug = keys[SDL_SCANCODE_SCROLLLOCK];
  }

  void draw()
  {
    m_display->beginDraw();

    auto actors = m_scene->getActors();

    for(auto& actor : actors)
    {
      auto where = Rect2f(actor.pos.x, actor.pos.y, actor.scale.width, actor.scale.height);
      m_display->drawActor(where, !actor.screenRefFrame, (int)actor.model, actor.effect == Effect::Blinking, actor.action, actor.ratio);
    }

    if(m_paused)
      m_display->drawText(Vector2f(0, 0), "PAUSE");
    else if(m_slowMotion)
      m_display->drawText(Vector2f(0, 0), "SLOW-MOTION MODE");
    else if(m_control.debug)
      m_display->drawText(Vector2f(0, 0), "DEBUG MODE");

    if(m_textboxDelay > 0)
    {
      m_display->drawText(Vector2f(0, 2), m_textbox.c_str());
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

  void onKeyDown(SDL_Event* evt)
  {
    if(evt->key.keysym.sym == SDLK_ESCAPE)
      onQuit();

    if(evt->key.keysym.sym == SDLK_F2)
      m_scene.reset(createGame(this, m_args));

    if(evt->key.keysym.sym == SDLK_TAB)
      m_slowMotion = !m_slowMotion;

    if(evt->key.keysym.sym == SDLK_RETURN && (evt->key.keysym.mod & KMOD_LALT))
    {
      if(evt->key.repeat == 0)
      {
        m_fullscreen = !m_fullscreen;
        m_display->setFullscreen(m_fullscreen);
      }
    }
    else if(evt->key.keysym.sym == SDLK_PAUSE)
    {
      m_audio->playSound(0);
      m_paused = !m_paused;
    }

    keys[evt->key.keysym.scancode] = 1;
  }

  void onKeyUp(SDL_Event* evt)
  {
    keys[evt->key.keysym.scancode] = 0;
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

  int keys[SDL_NUM_SCANCODES] {};
  int m_running = 1;

  int m_lastTime;
  int m_lastFps = 0;
  RateCounter m_fps;
  Control m_control;
  vector<string> m_args;
  unique_ptr<Scene> m_scene;
  bool m_slowMotion = false;
  bool m_fullscreen = false;
  bool m_paused = false;
  unique_ptr<Audio> m_audio;
  unique_ptr<Display> m_display;

  string m_title;
  string m_textbox;
  int m_textboxDelay = 0;
};

///////////////////////////////////////////////////////////////////////////////

App* App_create(vector<string> argv)
{
  return new App(argv);
}

void App_destroy(App* app)
{
  delete app;
}

bool App_tick(App* app)
{
  return app->tick();
}

