/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// splash menu

#include "base/scene.h"
#include "base/view.h"

#include "vec.h"
#include "toggle.h"
#include "models.h" // MDL_SPLASH
#include "state_machine.h"

struct SplashState : Scene
{
  SplashState(View* view_, StateMachine* fsm_) : view(view_), fsm(fsm_)
  {
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  void tick(Control const& c) override
  {
    view->playMusic(6);

    if(!activated)
    {
      delay = 1000;

      if(c.fire || c.jump || c.dash)
        activated = true;
    }

    view->setCameraPos(NullVector);
    view->setAmbientLight(delay / 1000.0 - 1.0);

    if(activated)
    {
      if(decrement(delay))
      {
        activated = false;
        fsm->next();
      }
    }
  }

  vector<Actor> getActors() const override
  {
    auto splash = Actor(NullVector, MDL_SPLASH);
    splash.scale = Size2f(16, 16);
    splash.pos -= Vector2f(8, 8);
    return vector<Actor>({ splash });
  }

private:
  View* const view;
  StateMachine* const fsm;
  bool activated = false;
  int delay = 0;
};

unique_ptr<Scene> createSplashState(StateMachine* fsm, View* view)
{
  return make_unique<SplashState>(view, fsm);
}

