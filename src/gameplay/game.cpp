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

namespace
{
struct RootScene : Scene
{
  RootScene(IRenderer* renderer, Audio* audio, Span<const std::string> args)
  {
    m_presenter.reset(createPresenter(renderer, audio));

    for(auto res : getResources())
      m_presenter->preload(res);

    if(args.len == 1)
    {
      int level = atoi(args[0].c_str());
      m_scene.reset(createPlayingStateAtLevel(m_presenter.get(), level));
    }
    else
    {
      m_scene.reset(createBootupState(m_presenter.get()));
    }
  }

  Scene* tick(Control c) override
  {
    auto next = m_scene->tick(c);

    if(next != m_scene.get())
    {
      m_scene.release();
      m_scene.reset(next);
      printf("Entering scene: %s\n", typeid(*next).name());
    }

    return nullptr;
  }

  void draw() override
  {
    m_scene->draw();
    m_presenter->flushFrame();
  }

  std::unique_ptr<IPresenter> m_presenter;
  std::unique_ptr<Scene> m_scene;
};
}

Scene* createGame(IRenderer* renderer, Audio* audio, Span<const std::string> args)
{
  return new RootScene(renderer, audio, args);
}

