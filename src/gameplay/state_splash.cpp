// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// splash menu

#include "base/scene.h"
#include <memory>

#include "models.h" // MDL_SPLASH
#include "presenter.h"
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

struct SplashState : Scene
{
  SplashState(IPresenter* view_) : view(view_)
  {
    startButton.toggle(true);
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  Scene* tick(Control c) override
  {
    auto const FADE_TIME = 100;

    if(!activated)
    {
      view->playMusic(6);

      delay = FADE_TIME;

      if(startButton.toggle(c.fire || c.jump || c.dash || c.start))
      {
        view->stopMusic();
        activated = true;
      }
    }

    view->setCameraPos(NullVector);
    view->setAmbientLight(delay / float(FADE_TIME) - 1.0);

    if(activated)
    {
      if(decrement(delay))
      {
        std::unique_ptr<Scene> deleteMeOnReturn(this);
        return createPlayingState(view);
      }
    }

    return this;
  }

  void draw() override
  {
    {
      auto splash = SpriteActor { NullVector, MDL_SPLASH };
      splash.scale = { 15, 10 };
      view->sendActor(splash);
    }
  }

private:
  IPresenter* const view;
  Toggle startButton;
  bool activated = false;
  int delay = 0;
};

Scene* createSplashState(IPresenter* view)
{
  return new SplashState(view);
}

