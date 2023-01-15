// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// minimap paused state

#include "base/scene.h"
#include <memory>

#include "minimap_data.h"
#include "models.h" // MDL_PAUSED
#include "presenter.h"
#include "quest.h"
#include "sounds.h"
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

struct PausedState : Scene
{
  PausedState(IPresenter* view_, Scene* sub_, const MinimapData& minimapData) : view(view_), sub(sub_), quest(minimapData.quest), m_roomIdx(minimapData.level)
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

      if(room.size.x == 1 && room.size.y == 1)
      {
        auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
        cell.action = 1;
        cell.scale.x = cellSize;
        cell.scale.y = cellSize;
        cell.pos.x = cellSize * origin.x;
        cell.pos.y = cellSize * origin.y;
        cell.screenRefFrame = true;
        cell.zOrder = 11;
        view->sendActor(cell);
      }
      else if(room.size.y == 1) // horizontal corridor
      {
        for(int i = 0; i < room.size.x; ++i)
        {
          auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
          cell.action = i == 0 ? 10 : (i == room.size.x - 1 ? 11 : 13);
          cell.scale.x = cellSize;
          cell.scale.y = cellSize;
          cell.pos.x = cellSize * (origin.x + i);
          cell.pos.y = cellSize * origin.y;
          cell.screenRefFrame = true;
          cell.zOrder = 11;
          view->sendActor(cell);
        }
      }
      else if(room.size.x == 1) // vertical corridor
      {
        for(int i = 0; i < room.size.y; ++i)
        {
          auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
          cell.action = i == 0 ? 15 : (i == room.size.y - 1 ? 14 : 12);
          cell.scale.x = cellSize;
          cell.scale.y = cellSize;
          cell.pos.x = cellSize * origin.x;
          cell.pos.y = cellSize * (origin.y + i);
          cell.screenRefFrame = true;
          cell.zOrder = 11;
          view->sendActor(cell);
        }
      }
      else
      {
        for(int x = 0; x < room.size.x; ++x)
          for(int y = 0; y < room.size.y; ++y)
          {
            auto cell = Actor { NullVector, MDL_MINIMAP_TILES };

            if(x == 0)
            {
              if(y == 0)
                cell.action = 4;
              else if(y == room.size.y - 1)
                cell.action = 2;
              else
                cell.action = 6;
            }
            else if(x == room.size.x - 1)
            {
              if(y == 0)
                cell.action = 5;
              else if(y == room.size.y - 1)
                cell.action = 3;
              else
                cell.action = 8;
            }
            else if(y == 0)
            {
              cell.action = 7;
            }
            else if(y == room.size.y - 1)
            {
              cell.action = 9;
            }

            cell.scale.x = cellSize;
            cell.scale.y = cellSize;
            cell.pos.x = cellSize * (x + origin.x);
            cell.pos.y = cellSize * (y + origin.y);
            cell.screenRefFrame = true;
            cell.zOrder = 11;
            view->sendActor(cell);
          }
      }
    }

    {
      auto& currRoom = quest->rooms[m_roomIdx];
      auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
      cell.action = 17;
      cell.scale.x = cellSize * currRoom.size.x;
      cell.scale.y = cellSize * currRoom.size.y;
      cell.pos.x = cellSize * (currRoom.pos.x - m_scroll.x);
      cell.pos.y = cellSize * (currRoom.pos.y - m_scroll.y);
      cell.screenRefFrame = true;
      cell.zOrder = 12;
      cell.effect = Effect::Blinking;
      view->sendActor(cell);
    }

    auto overlay = Actor { NullVector, MDL_MINIMAP_BG };
    overlay.scale = { 16, 16 };
    overlay.pos -= Vec2f(8, 8);
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
  const Quest* const quest;
  int const m_roomIdx;
  Vec2i m_scroll {};
};

Scene* createPausedState(IPresenter* view, Scene* sub, const MinimapData& minimapData)
{
  return new PausedState(view, sub, minimapData);
}

