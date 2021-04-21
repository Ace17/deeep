// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Entry point.
// This is the only file where emscripten-specific stuff can appear.

#include "base/error.h"
#include <cstdio>

#include "app.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"

using namespace std;

#ifdef __EMSCRIPTEN__
extern "C"
{
void emscripten_set_main_loop(void (* f)(), int, int);
}

static IApp* g_theApp;

static void tickTheApp()
{
  g_theApp->tick();
}

void runMainLoop(IApp* app)
{
  g_theApp = app;
  emscripten_set_main_loop(&tickTheApp, 0, 10);
}

#else

void runMainLoop(IApp* app)
{
  while(app->tick())
    SDL_Delay(1);
}

#endif

int main(int argc, char* argv[])
{
  try
  {
    auto app = createApp({ argv + 1, argc - 1 });
    runMainLoop(app.get());
    return 0;
  }
  catch(Error const& e)
  {
    const auto msg = e.message();
    fflush(stdout);
    fprintf(stderr, "Fatal: %.*s\n", msg.len, msg.data);
    return 1;
  }
}

