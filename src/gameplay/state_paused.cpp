// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// minimap paused state

#include "base/scene.h"
#include "misc/math.h"
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
    m_scroll = -currRoom.pos;
    m_scroll.x -= int(minimapData.playerPos.x) / CELL_SIZE.x;
    m_scroll.y -= int(minimapData.playerPos.y) / CELL_SIZE.y;
    m_scrollf = Vec2f(m_scroll.x, m_scroll.y);

    mapViewModel = computeMapViewModel(minimapData);
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
      m_scroll.x++;

    if(rightButton.toggle(c.right))
      m_scroll.x--;

    if(upButton.toggle(c.up))
      m_scroll.y--;

    if(downButton.toggle(c.down))
      m_scroll.y++;

    const Vec2f target(m_scroll.x, m_scroll.y);
    m_scrollf = lerp(m_scrollf, target, 0.2);

    return this;
  }

  void draw() override
  {
    drawMinimap(view, m_scrollf, mapViewModel);

    auto overlay = SpriteActor { NullVector, MDL_MINIMAP_BG };
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
  MapViewModel mapViewModel{};
  IPresenter* const view;
  std::unique_ptr<Scene> sub;
  const MinimapData minimapData;
  const Quest* const quest;
  Vec2i m_scroll {};
  Vec2f m_scrollf {};
};

Scene* createPausedState(IPresenter* view, Scene* sub, const MinimapData& minimapData)
{
  return new PausedState(view, sub, minimapData);
}

