// Copyright (C) 2024 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/util.h"
#include "entity_factory.h" // getEntityFlags
#include "minimap_data.h"
#include "models.h" // MDL_MINIMAP_TILES
#include "presenter.h"
#include "quest.h"
#include "vec.h"

#include <algorithm>

namespace
{
int getOverlayTile(MapViewModel::CenterType center)
{
  switch(center)
  {
  case MapViewModel::CenterType::Item:
    return 15;
  case MapViewModel::CenterType::Save:
    return 14;
  default:
    return 13;
  }
}
}

void drawMinimap(IPresenter* presenter, Vec2f pos, const MapViewModel& vm)
{
  static auto const cellSize = 1.0;

  // minimap tiles
  for(int y = 0; y < vm.cells.size.y; ++y)
  {
    for(int x = 0; x < vm.cells.size.x; ++x)
    {
      const auto& cell = vm.cells.get(x, y);

      if(cell.center == MapViewModel::CenterType::Solid)
        continue;

      const MapViewModel::EdgeType wallRight = x == vm.cells.size.x - 1 ? MapViewModel::EdgeType::Wall : cell.right;
      const MapViewModel::EdgeType wallUp = y == vm.cells.size.y - 1 ? MapViewModel::EdgeType::Wall : cell.up;
      const MapViewModel::EdgeType wallDown = y == 0 ? MapViewModel::EdgeType::Wall : vm.cells.get(x, y - 1).up;
      const MapViewModel::EdgeType wallLeft = x == 0 ? MapViewModel::EdgeType::Wall : vm.cells.get(x - 1, y).right;

      auto actor = SpriteActor { NullVector, MDL_MINIMAP_TILES };
      actor.pos.x = cellSize * x + pos.x * cellSize;
      actor.pos.y = cellSize * y + pos.y * cellSize;
      actor.scale.x = cellSize;
      actor.scale.y = cellSize;
      actor.screenRefFrame = true;
      actor.zOrder = 11;

      actor.action = cell.visited ? 1 : 0;
      presenter->sendActor(actor);

      actor.zOrder = 13;

      if(wallLeft != MapViewModel::EdgeType::Free)
      {
        actor.action = 4;

        if(wallLeft == MapViewModel::EdgeType::Door)
          actor.action += 4;

        presenter->sendActor(actor);
      }

      if(wallRight != MapViewModel::EdgeType::Free)
      {
        actor.action = 5;

        if(wallRight == MapViewModel::EdgeType::Door)
          actor.action += 4;

        presenter->sendActor(actor);
      }

      if(wallUp != MapViewModel::EdgeType::Free)
      {
        actor.action = 6;

        if(wallUp == MapViewModel::EdgeType::Door)
          actor.action += 4;

        presenter->sendActor(actor);
      }

      if(wallDown != MapViewModel::EdgeType::Free)
      {
        actor.action = 7;

        if(wallDown == MapViewModel::EdgeType::Door)
          actor.action += 4;

        presenter->sendActor(actor);
      }

      // overlay (item, save)
      if(cell.center != MapViewModel::CenterType::Hollow)
      {
        actor.action = getOverlayTile(cell.center);
        actor.zOrder = 14;
        presenter->sendActor(actor);
      }
    }
  }

  // player
  {
    auto actor = SpriteActor { NullVector, MDL_MINIMAP_TILES };
    actor.action = 13;
    actor.pos.x = cellSize * (vm.playerPos.x + pos.x);
    actor.pos.y = cellSize * (vm.playerPos.y + pos.y);
    actor.scale.x = cellSize;
    actor.scale.y = cellSize;
    actor.screenRefFrame = true;
    actor.zOrder = 12;
    actor.effect = Effect::Blinking;

    presenter->sendActor(actor);
  }
}

extern const Vec2i CELL_SIZE;

MapViewModel computeMapViewModel(const MinimapData& map)
{
  MapViewModel r;

  Vec2i mapSize {};

  for(auto& r : map.quest->rooms)
  {
    const Vec2i roomTopRight = r.pos + r.size;
    mapSize.x = std::max(mapSize.x, roomTopRight.x);
    mapSize.y = std::max(mapSize.y, roomTopRight.y);
  }

  r.cells.resize(mapSize);

  for(auto& room : map.quest->rooms)
  {
    for(auto pair : rasterScan(room.size.x, room.size.y))
    {
      Vec2i pos = room.pos + Vec2i(pair.first, pair.second);
      auto& cell = r.cells.get(pos.x, pos.y);
      cell.center = MapViewModel::CenterType::Hollow;
      cell.up = cell.right = MapViewModel::EdgeType::Free;
    }

    // horizontal walls
    for(int x = 0; x < room.size.x; ++x)
    {
      // top line
      r.cells.get(room.pos.x + x, room.pos.y + room.size.y - 1).up = MapViewModel::EdgeType::Wall;
      // bottom line
      r.cells.get(room.pos.x + x, room.pos.y - 1).up = MapViewModel::EdgeType::Wall;
    }

    // vertical walls
    for(int y = 0; y < room.size.y; ++y)
    {
      // left line
      r.cells.get(room.pos.x - 1, room.pos.y + y).right = MapViewModel::EdgeType::Wall;
      // right line
      r.cells.get(room.pos.x + room.size.x - 1, room.pos.y + y).right = MapViewModel::EdgeType::Wall;
    }

    for(auto& spawner : room.spawners)
    {
      const int flags = getEntityFlags(spawner.name);

      const int x = room.pos.x + spawner.pos.x / CELL_SIZE.x;
      const int y = room.pos.y + spawner.pos.y / CELL_SIZE.y;

      if(flags & EntityFlag_ShowOnMinimap_S)
        r.cells.get(x, y).center = MapViewModel::CenterType::Save;

      if(flags & EntityFlag_ShowOnMinimap_O)
        r.cells.get(x, y).center = MapViewModel::CenterType::Item;
    }
  }

  r.playerPos = map.quest->rooms[map.level].pos;
  r.playerPos.x += int(map.playerPos.x) / CELL_SIZE.x;
  r.playerPos.y += int(map.playerPos.y) / CELL_SIZE.y;

  auto hideCellIfUnknown =
    [&] (int x, int y, MapViewModel::Cell& cell)
    {
      int status = map.exploredCells->get(x, y);
      switch(status)
      {
      case 0:
        cell.center = MapViewModel::CenterType::Solid;
        break;
      case 1:
        cell.visited = false;
        break;
      case 2:
        cell.visited = true;
        break;
      }
    };

  r.cells.scan(hideCellIfUnknown);

  return r;
}

