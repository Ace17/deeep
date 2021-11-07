// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// main FSM: dispatching between various game states

#include "base/span.h"
#include "base/view.h"
#include "state_machine.h"
#include <string>

using namespace std;

Span<const Resource> getResources();

Scene* createGame(View* view, Span<const string> args)
{
  view->setTitle("Deeep");

  for(auto res : getResources())
    view->preload(res);

  if(args.len == 1)
  {
    int level = atoi(args[0].c_str());
    return createPlayingStateAtLevel(view, level);
  }

  return createSplashState(view);
}

