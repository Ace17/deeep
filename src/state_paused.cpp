// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// minimap paused state

#include "base/scene.h"
#include "base/view.h"
#include <memory>

#include "models.h" // MDL_PAUSED
#include "quest.h"
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

struct PausedState : Scene
{
  PausedState(View* view_, Scene* sub_, Quest* quest_, int roomIdx) : view(view_), sub(sub_), quest(quest_), m_roomIdx(roomIdx)
  {
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  Scene* tick(Control c) override
  {
    decrement(pauseDelay);

    if(startButton.toggle(c.start) && !pauseDelay)
    {
      std::unique_ptr<Scene> deleteMeOnReturn(this);
      return sub.release();
    }

    return this;
  }

  void draw() override
  {
    sub->draw();

    for(int idx = 0; idx < (int)quest->rooms.size(); ++idx)
    {
      auto& room = quest->rooms[idx];

      auto const cellSize = 0.4;
      int col = room.pos.x;
      int row = room.pos.y;

      auto cell = Actor { NullVector, MDL_RECT };
      cell.action = 5;
      cell.scale.width = room.size.width * cellSize;
      cell.scale.height = room.size.height * cellSize;
      cell.pos.x = cellSize * (col - 20);
      cell.pos.y = cellSize * (row - 24);
      cell.screenRefFrame = true;
      cell.zOrder = 5;

      if(idx == m_roomIdx)
        cell.action = 0;

      view->sendActor(cell);
    }

    auto overlay = Actor { NullVector, MDL_PAUSED };
    overlay.scale = Size2f(12, 12);
    overlay.pos -= Vector2f(6, 6);
    overlay.screenRefFrame = true;
    overlay.zOrder = 4;
    view->sendActor(overlay);
  }

private:
  int pauseDelay = 10;
  Toggle startButton;
  View* const view;
  std::unique_ptr<Scene> sub;
  Quest* const quest;
  int const m_roomIdx;
};

Scene* createPausedState(View* view, Scene* sub, Quest* quest, int roomIdx)
{
  return new PausedState(view, sub, quest, roomIdx);
}

