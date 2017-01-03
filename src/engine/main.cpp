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
#include <chrono>
#include <thread>
#include <string>
#include <vector>

#include "SDL.h"

using namespace std;

class App;
App* App_create(vector<string> args);
bool App_tick(App*);

#ifdef __EMSCRIPTEN__
extern "C"
{
void emscripten_set_main_loop(void (* f)(), int, int);
char const* gluErrorString(int)
{
  return "unknown";
}
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
    vector<string> args;

    for(int i = 1; i < argc; ++i)
      args.push_back(argv[i]);

    auto app = App_create(args);

    runMainLoop(app);
    return 0;
  }
  catch(exception const& e)
  {
    cerr << "Fatal: " << e.what() << endl;
    return 1;
  }
}

