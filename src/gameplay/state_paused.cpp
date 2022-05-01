// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// minimap paused state

#include "base/scene.h"
#include <memory>

#include "models.h" // MDL_PAUSED
#include "presenter.h"
#include "quest.h"
#include "sounds.h"
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

struct PausedState : Scene
{
  PausedState(IPresenter* view_, Scene* sub_, Quest* quest_, int roomIdx) : view(view_), sub(sub_), quest(quest_), m_roomIdx(roomIdx)
  {
    view->playSound(SND_PAUSE);

    auto& currRoom = quest->rooms[m_roomIdx];
    m_scroll = currRoom.pos;
  }

  ////////////////////////////////////////////////////////////////
  // Scene: Game, seen by the engine

  Scene* tick(Control c) override
  {
    decrement(pauseDelay);

    if(startButton.toggle(c.start) && !pauseDelay)
    {
      view->playSound(SND_PAUSE);
      std::unique_ptr<Scene> deleteMeOnReturn(this);
      return sub.release();
    }

    if(leftButton.toggle(c.left))
      m_scroll.x--;

    if(rightButton.toggle(c.right))
      m_scroll.x++;

    if(upButton.toggle(c.up))
      m_scroll.y++;

    if(downButton.toggle(c.down))
      m_scroll.y--;

    return this;
  }

  void draw() override
  {
    static auto const cellSize = 1.0;

    for(int idx = 0; idx < (int)quest->rooms.size(); ++idx)
    {
      auto& room = quest->rooms[idx];

      const auto origin = room.pos - m_scroll;

      if(room.size.width == 1 && room.size.height == 1)
      {
        auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
        cell.action = 1;
        cell.scale.width = cellSize;
        cell.scale.height = cellSize;
        cell.pos.x = cellSize * origin.x;
        cell.pos.y = cellSize * origin.y;
        cell.screenRefFrame = true;
        cell.zOrder = 11;
        view->sendActor(cell);
      }
      else if(room.size.height == 1) // horizontal corridor
      {
        for(int i = 0; i < room.size.width; ++i)
        {
          auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
          cell.action = i == 0 ? 10 : (i == room.size.width - 1 ? 11 : 13);
          cell.scale.width = cellSize;
          cell.scale.height = cellSize;
          cell.pos.x = cellSize * (origin.x + i);
          cell.pos.y = cellSize * origin.y;
          cell.screenRefFrame = true;
          cell.zOrder = 11;
          view->sendActor(cell);
        }
      }
      else if(room.size.width == 1) // vertical corridor
      {
        for(int i = 0; i < room.size.height; ++i)
        {
          auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
          cell.action = i == 0 ? 15 : (i == room.size.height - 1 ? 14 : 12);
          cell.scale.width = cellSize;
          cell.scale.height = cellSize;
          cell.pos.x = cellSize * origin.x;
          cell.pos.y = cellSize * (origin.y + i);
          cell.screenRefFrame = true;
          cell.zOrder = 11;
          view->sendActor(cell);
        }
      }
      else
      {
        for(int x = 0; x < room.size.width; ++x)
          for(int y = 0; y < room.size.height; ++y)
          {
            auto cell = Actor { NullVector, MDL_MINIMAP_TILES };

            if(x == 0)
            {
              if(y == 0)
                cell.action = 4;
              else if(y == room.size.height - 1)
                cell.action = 2;
              else
                cell.action = 6;
            }
            else if(x == room.size.width - 1)
            {
              if(y == 0)
                cell.action = 5;
              else if(y == room.size.height - 1)
                cell.action = 3;
              else
                cell.action = 8;
            }
            else if(y == 0)
            {
              cell.action = 7;
            }
            else if(y == room.size.height - 1)
            {
              cell.action = 9;
            }

            cell.scale.width = cellSize;
            cell.scale.height = cellSize;
            cell.pos.x = cellSize * (x + origin.x);
            cell.pos.y = cellSize * (y + origin.y);
            cell.screenRefFrame = true;
            cell.zOrder = 11;
            view->sendActor(cell);
          }
      }
    }

    auto overlay = Actor { NullVector, MDL_MINIMAP_BG };
    overlay.scale = Size2f(16, 16);
    overlay.pos -= Vector2f(8, 8);
    overlay.screenRefFrame = true;
    overlay.zOrder = 12;
    view->sendActor(overlay);
  }

private:
  int pauseDelay = 10;
  Toggle startButton;
  Toggle leftButton;
  Toggle rightButton;
  Toggle upButton;
  Toggle downButton;
  IPresenter* const view;
  std::unique_ptr<Scene> sub;
  Quest* const quest;
  int const m_roomIdx;
  Vector2i m_scroll {};
};

Scene* createPausedState(IPresenter* view, Scene* sub, Quest* quest, int roomIdx)
{
  return new PausedState(view, sub, quest, roomIdx);
}

