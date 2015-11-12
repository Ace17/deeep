/**
 * @brief Entry point.
 * This is the only file where emscripten-specific stuff can appear.
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <iostream>
#include <stdexcept>
#include "SDL.h"
#include "util.h"
#include "scene.h"

using namespace std;

class App;
App* App_create(Scene*);
bool App_tick(App*);

void Display_init(int width, int height);
void Display_loadModel(int id, const char* imagePath);

void Audio_init();
void Audio_loadSound(int id, string path);

#ifdef __EMSCRIPTEN__
extern "C"
{
void emscripten_set_main_loop(void (* f)(), int, int);
char const* gluErrorString(int)
{
  return "unknown";
}
}

App* g_pApp;
static void voidTick()
{
  App_tick(g_pApp);
}

void runMainLoop(App* app)
{
  g_pApp = app;
  emscripten_set_main_loop(&voidTick, 0, 1);
}

#else

void runMainLoop(App* app)
{
  while(App_tick(app))
    SDL_Delay(1);
}

#endif

Scene* createGame();

int main()
{
  try
  {
    auto game = unique(createGame());
    auto app = App_create(game.get());

    Display_init(512, 512);
    Audio_init();

    auto const sounds = game->getSounds();

    for(int i = 0; sounds[i].path; ++i)
      Audio_loadSound(sounds[i].id, sounds[i].path);

    auto const models = game->getModels();

    for(int i = 0; models[i].path; ++i)
      Display_loadModel(models[i].id, models[i].path);

    runMainLoop(app);
    return 0;
  }
  catch(exception const& e)
  {
    cerr << "Fatal: " << e.what() << endl;
    return 1;
  }
}

