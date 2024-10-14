// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// splash menu

#include "base/scene.h"
#include <algorithm>
#include <memory>

#include "models.h" // MDL_BOOTUP
#include "presenter.h"
#include "sounds.h" // SND_STARTUP
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

static auto const BEEPTIME = 190;
static auto const BOOTUPTIME = BEEPTIME + 90;

struct BootupState : Scene
{
  BootupState(IPresenter* view_) : view(view_)
  {
    timer = 0;
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  Scene* tick(Control) override
  {
    view->stopMusic();
    view->setCameraPos(NullVector);

    ++timer;

    if(timer == BEEPTIME)
      view->playSound(SND_STARTUP);

    if(timer == BOOTUPTIME)
    {
      std::unique_ptr<Scene> deleteMeOnReturn(this);
      return createSplashState(view);
    }

    return this;
  }

  void draw() override
  {
    float ratio = std::min(timer / float(BEEPTIME), 1.0f);
    auto splash = SpriteActor { NullVector, MDL_BOOTUP };
    splash.scale = { 16, 16 };
    splash.pos.y = 0;
    splash.zOrder = 2;
    view->sendActor(splash);
    view->setAmbientLight(1 - ratio);
  }

private:
  IPresenter* const view;
  int timer = 0;
};

Scene* createBootupState(IPresenter* view)
{
  return new BootupState(view);
}

