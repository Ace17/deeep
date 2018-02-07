/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Entry point.
// This is the only file where emscripten-specific stuff can appear.

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

#define SDL_MAIN_HANDLED
#include "SDL.h"

using namespace std;

class App;
App* App_create(vector<string> args);
bool App_tick(App*);
void App_destroy(App* app);

#ifdef __EMSCRIPTEN__
extern "C"
{
void emscripten_set_main_loop(void (* f)(), int, int);
}

static App* g_pApp;
static void voidTick()
{
  App_tick(g_pApp);
}

void runMainLoop(App* app)
{
  g_pApp = app;
  emscripten_set_main_loop(&voidTick, 0, 10);
}

#else

void runMainLoop(App* app)
{
  while(App_tick(app))
    SDL_Delay(10);
}

#endif

int main(int argc, char* argv[])
{
  try
  {
    vector<string> args {
      argv + 1, argv + argc
    };

    auto app = App_create(args);

    runMainLoop(app);
    App_destroy(app);
    return 0;
  }
  catch(exception const& e)
  {
    cerr << "Fatal: " << e.what() << endl;
    return 1;
  }
}

