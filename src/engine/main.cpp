// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Entry point.
// This is the only file where emscripten-specific stuff can appear.

#include "base/error.h"
#include "base/logger.h"

#include "app.h"

#define SDL_MAIN_HANDLED
#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static IApp* g_theApp;

static void tickTheApp()
{
  g_theApp->tick();
}

void runMainLoop(IApp* app)
{
  g_theApp = app;
  emscripten_set_main_loop(&tickTheApp, -1, 1);
}

#else

void runMainLoop(IApp* app)
{
  while(app->tick())
    SDL_Delay(1);
}

#endif

#ifdef __linux__
#include <sys/resource.h>
#endif

int main(int argc, char* argv[])
{
  void foo(void);

#ifdef __linux__
  rlimit lim { 8192, 8192 };
  setrlimit(RLIMIT_STACK, &lim);
#endif

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
    logMsg("Fatal: %.*s", msg.len, msg.data);
    return 1;
  }
}

