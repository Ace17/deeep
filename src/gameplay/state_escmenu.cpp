// Copyright (C) 2026 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// ingame escape menu

#include "base/scene.h"
#include "misc/math.h"
#include <memory>

#include "models.h"
#include "presenter.h"
#include "sounds.h"
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

extern const Vec2i CELL_SIZE;

struct EscapeMenuState : Scene
{
  EscapeMenuState(IPresenter* view_, Scene* sub_)
    : view(view_),
    sub(sub_)
  {
    view->playSound(SND_PAUSE);
  }

  Scene* tick(Control c) override
  {
    if(c.menu == Control::JustPressed)
    {
      view->playSound(SND_PAUSE);
      std::unique_ptr<Scene> deleteMeOnReturn(this);
      return sub.release();
    }

    if(c.start == Control::JustPressed)
    {
      std::unique_ptr<Scene> deleteMeOnReturn(this);

      if(selection == 0)
        return sub.release(); // return to game
      else if(selection == 1)
        return createSplashState(view); // quit to title
      else
        return &nullScene; // exit program
    }

    if(c.down == Control::JustPressed)
    {
      selection++;
      selection %= 3;
    }

    if(c.up == Control::JustPressed)
    {
      selection = (selection - 1 + 3) % 3;
    }

    return this;
  }

  void draw() override
  {
    sub->draw();

    auto overlay = SpriteActor { NullVector, MDL_ENGINE };
    overlay.scale = { 15, 10 };
    overlay.screenRefFrame = true;
    overlay.zOrder = 20;
    view->sendActor(overlay);

    for(int i = 0; i < 3; ++i)
    {
      auto menuChoice = TileActor { { { -4, -i * 1.25f }, { 8, 0.9 } }, MDL_RECT };
      menuChoice.screenRefFrame = true;
      menuChoice.zOrder = 20;

      if(i == selection)
        menuChoice.action = 0;
      else
        menuChoice.action = 1;

      view->sendActor(menuChoice);
    }
  }

private:
  int selection = 0;
  IPresenter* const view;
  std::unique_ptr<Scene> sub;
};

Scene* createEscapeMenuState(IPresenter* view, Scene* sub)
{
  return new EscapeMenuState(view, sub);
}

