// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// minimap paused state

#include "base/scene.h"
#include <memory>

#include "entity_factory.h"
#include "minimap_data.h"
#include "models.h" // MDL_PAUSED
#include "presenter.h"
#include "quest.h"
#include "sounds.h"
#include "state_machine.h"
#include "toggle.h"
#include "vec.h"

extern const Vec2i CELL_SIZE;

struct PausedState : Scene
{
  PausedState(IPresenter* view_, Scene* sub_, const MinimapData& minimapData_) : view(view_), sub(sub_), minimapData(minimapData_), quest(minimapData_.quest)
  {
    view->playSound(SND_PAUSE);

    auto& currRoom = quest->rooms[minimapData.level];
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

    // minimap tiles
    for(int y = 0; y < quest->minimapTiles.size.y; ++y)
    {
      for(int x = 0; x < quest->minimapTiles.size.x; ++x)
      {
        const int tile = quest->minimapTiles.get(x, y);

        if(tile < 0)
          continue;

        auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
        cell.action = tile;
        cell.pos.x = cellSize * (x - m_scroll.x);
        cell.pos.y = cellSize * (y - m_scroll.y);
        cell.scale.x = cellSize;
        cell.scale.y = cellSize;
        cell.screenRefFrame = true;
        cell.zOrder = 11;
        view->sendActor(cell);
      }
    }

    // minimap overlays (savepoints, items, etc.)
    for(auto& room : quest->rooms)
    {
      for(auto& spawner : room.spawners)
      {
        int flags = getEntityFlags(spawner.name);
        int tile = 0;

        if(flags & EntityFlag_ShowOnMinimap_S)
          tile = 18;

        if(flags & EntityFlag_ShowOnMinimap_O)
          tile = 19;

        if(tile)
        {
          int x = room.pos.x + spawner.pos.x / CELL_SIZE.x;
          int y = room.pos.y + spawner.pos.y / CELL_SIZE.y;
          auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
          cell.action = tile;
          cell.pos.x = cellSize * (x - m_scroll.x);
          cell.pos.y = cellSize * (y - m_scroll.y);
          cell.scale.x = cellSize;
          cell.scale.y = cellSize;
          cell.screenRefFrame = true;
          cell.zOrder = 12;
          view->sendActor(cell);
        }
      }
    }

    {
      auto& currRoom = quest->rooms[minimapData.level];

      Vec2i playerCell;
      playerCell.x = currRoom.pos.x + int(minimapData.playerPos.x) / CELL_SIZE.x;
      playerCell.y = currRoom.pos.y + int(minimapData.playerPos.y) / CELL_SIZE.y;

      auto cell = Actor { NullVector, MDL_MINIMAP_TILES };
      cell.action = 17;
      cell.scale.x = cellSize;
      cell.scale.y = cellSize;
      cell.pos.x = cellSize * (playerCell.x - m_scroll.x);
      cell.pos.y = cellSize * (playerCell.y - m_scroll.y);
      cell.screenRefFrame = true;
      cell.zOrder = 12;
      cell.effect = Effect::Blinking;
      view->sendActor(cell);
    }

    auto overlay = Actor { NullVector, MDL_MINIMAP_BG };
    overlay.scale = { 16, 16 };
    overlay.pos -= Vec2f(8, 8);
    overlay.screenRefFrame = true;
    overlay.zOrder = 13;
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
  const MinimapData minimapData;
  const Quest* const quest;
  Vec2i m_scroll {};
};

Scene* createPausedState(IPresenter* view, Scene* sub, const MinimapData& minimapData)
{
  return new PausedState(view, sub, minimapData);
}

