// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// splash menu

#include "base/scene.h"
#include "base/view.h"

#include "vec.h"
#include "toggle.h"
#include "models.h" // MDL_SPLASH
#include "state_machine.h"

struct PausedState : Scene
{
  PausedState(View* view_, Scene* sub_) : view(view_), sub(sub_)
  {
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  Scene* tick(Control c) override
  {
    decrement(pauseDelay);

    if(startButton.toggle(c.start) && !pauseDelay)
      return sub;

    return this;
  }

  void draw() override
  {
    sub->draw();

    auto overlay = Actor { NullVector, MDL_PAUSED };
    overlay.scale = Size2f(8, 8);
    overlay.pos -= Vector2f(4, 4);
    overlay.screenRefFrame = true;
    overlay.zOrder = 10;
    view->sendActor(overlay);
  }

private:
  int pauseDelay = 10;
  Toggle startButton;
  View* const view;
  Scene* const sub;
};

Scene* createPausedState(View* view, Scene* sub)
{
  return new PausedState(view, sub);
}

