/**
 * Main loop timing.
 * No game-specific code should be here,
 * and no platform-specific code should be here (SDL is OK).
 */

/*
 * Copyright (C) 2016 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include "SDL.h"
#include "base/geom.h"
#include "base/scene.h"
#include "ratecounter.h"
#include "sound.h"

using namespace std;

auto const TIMESTEP = 1;

void Display_init(int width, int height);
void Display_setCaption(const char* caption);
void Display_loadModel(int id, const char* imagePath);
void Display_beginDraw();
void Display_endDraw();
void Display_drawActor(Rect2f where, int modelId, bool blinking, int actionIdx, float frame);

Audio* createAudio(bool dummy = false);

Scene* createGame(vector<string> argv);

class App
{
public:
  App(vector<string> argv)
    : m_running(1),
    m_lastFps(0),
    m_args(argv),
    m_scene(createGame(m_args)),
    m_slowMotion(false)
  {
    SDL_Init(0);
    memset(keys, 0, sizeof keys);

    Display_init(768, 768);
    m_audio.reset(createAudio());

    for(auto sound : m_scene->getSounds())
      m_audio->loadSound(sound.id, sound.path);

    for(auto model : m_scene->getModels())
      Display_loadModel(model.id, model.path);

    m_lastTime = SDL_GetTicks();
  }

  ~App()
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
      {
        m_scene->tick(m_control);
        dirty = true;
      }
    }

    if(dirty)
    {
      draw();
      playSounds();
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
    m_control.fire = keys[SDL_SCANCODE_Z];
    m_control.jump = keys[SDL_SCANCODE_X];
    m_control.dash = keys[SDL_SCANCODE_C];

    m_control.debug = keys[SDL_SCANCODE_G];
  }

  void draw()
  {
    Display_beginDraw();

    for(auto& actor : m_scene->getActors())
    {
      auto where = Rect2f(actor.pos.x, actor.pos.y, actor.scale.x, actor.scale.y);
      Display_drawActor(where, (int)actor.model, actor.effect == EFFECT_BLINKING, actor.action, actor.ratio);
    }

    Display_endDraw();
  }

  void playSounds()
  {
    auto sounds = m_scene->readSounds();

    for(auto sound : sounds)
      m_audio->playSound(sound);

    m_audio->playMusic(m_scene->getMusic());
  }

  void fpsChanged(int fps)
  {
    char szFps[128];
    sprintf(szFps, "Deeep (%d FPS)", fps);
    Display_setCaption(szFps);
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
      m_scene.reset(createGame(m_args));

    if(evt->key.keysym.sym == SDLK_TAB)
      m_slowMotion = !m_slowMotion;

    if(evt->key.keysym.sym == SDLK_PAUSE || evt->key.keysym.sym == SDLK_RETURN)
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

  int keys[SDL_NUM_SCANCODES];
  int m_running;

  int m_lastTime;
  int m_lastFps;
  RateCounter m_fps;
  Control m_control;
  vector<string> m_args;
  unique_ptr<Scene> m_scene;
  bool m_slowMotion;
  Bool m_paused;
  unique_ptr<Audio> m_audio;
};

///////////////////////////////////////////////////////////////////////////////

App* App_create(vector<string> argv)
{
  return new App(argv);
}

bool App_tick(App* app)
{
  return app->tick();
}

