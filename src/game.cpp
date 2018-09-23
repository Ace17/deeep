// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// main FSM: dispatching between various game states

#include "state_machine.h"
#include "base/view.h"
#include "base/span.h"

unique_ptr<Scene> createSplashState(StateMachine* fsm, View* view);
unique_ptr<Scene> createGameState(StateMachine* fsm, View* view, int level);

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
  auto fsm = make_unique<StateMachine>();

  bool showSplash = true;
  int level = 1;

  if(args.size() == 1)
  {
    level = atoi(args[0].c_str());
    showSplash = false;
  }

  if(showSplash)
    fsm->states.push_back(createSplashState(fsm.get(), view));

  fsm->states.push_back(createGameState(fsm.get(), view, level));

  return fsm.release();
}

