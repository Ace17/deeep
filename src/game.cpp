// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// main FSM: dispatching between various game states

#include "state_machine.h"
#include "base/view.h"
#include "base/span.h"
#include <vector>
#include <string>

using namespace std;

Span<const Resource> getResources();

void preloadResources(View* view)
{
  for(auto res : getResources())
    view->preload(res);
}

Scene* createGame(View* view, vector<string> args)
{
  view->setTitle("Deeep");
  preloadResources(view);

  if(args.size() == 1)
  {
    int level = atoi(args[0].c_str());
    return createPlayingStateAtLevel(view, level);
  }

  return createSplashState(view);
}

