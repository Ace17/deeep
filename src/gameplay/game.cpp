// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// main FSM: dispatching between various game states

#include "base/audio.h"
#include "base/renderer.h"
#include "base/scene.h"
#include "base/span.h"
#include "presenter.h"
#include "state_machine.h"
#include <memory>
#include <string>

extern const String GAME_NAME = "Deeep";

using namespace std;

Span<const Resource> getResources();
IPresenter* createPresenter(IRenderer* renderer, Audio* audio);

std::unique_ptr<IPresenter> g_Presenter;

Scene* createGame(IRenderer* renderer, Audio* audio, Span<const std::string> args)
{
  g_Presenter.reset(createPresenter(renderer, audio));
  auto view = g_Presenter.get();

  for(auto res : getResources())
    view->preload(res);

  if(args.len == 1)
  {
    int level = atoi(args[0].c_str());
    return createPlayingStateAtLevel(view, level);
  }

  return createSplashState(view);
}

