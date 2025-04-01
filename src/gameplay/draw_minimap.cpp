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
    return 19;
  case MapViewModel::CenterType::Save:
    return 18;
  case MapViewModel::CenterType::Player:
    return 16;
  default:
    return 16;
  }
}

int getCenterTile(bool right, bool up, bool down, bool left)
{
  int code = 0;

  code <<= 1;
  code |= (int)right;
  code <<= 1;
  code |= (int)up;
  code <<= 1;
  code |= (int)down;
  code <<= 1;
  code |= (int)left;

  static const int tiles[16] =
  {
    /* RUDL */
    /* 0000 */ 0,
    /* 0001 */ 6,
    /* 0010 */ 7,
    /* 0011 */ 8,
    /* 0100 */ 11,
    /* 0101 */ 4,
    /* 0110 */ 3,
    /* 0111 */ 12,
    /* 1000 */ 10,
    /* 1001 */ 2,
    /* 1010 */ 9,
    /* 1011 */ 15,
    /* 1100 */ 5,
    /* 1101 */ 13,
    /* 1110 */ 14,
    /* 1111 */ 1,
  };

  return tiles[code];
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

      const bool wallRight = x == vm.cells.size.x - 1 || cell.right != MapViewModel::EdgeType::Free;
      const bool wallUp = y == vm.cells.size.y - 1 || cell.up != MapViewModel::EdgeType::Free;
      const bool wallDown = y == 0 || vm.cells.get(x, y - 1).up != MapViewModel::EdgeType::Free;
      const bool wallLeft = x == 0 || vm.cells.get(x - 1, y).right != MapViewModel::EdgeType::Free;

      auto actor = SpriteActor { NullVector, MDL_MINIMAP_TILES };
      actor.action = getCenterTile(wallRight, wallUp, wallDown, wallLeft) + (cell.visited ? 20 : 0);
      actor.pos.x = cellSize * x + pos.x * cellSize;
      actor.pos.y = cellSize * y + pos.y * cellSize;
      actor.scale.x = cellSize;
      actor.scale.y = cellSize;
      actor.screenRefFrame = true;
      actor.zOrder = 11;
      presenter->sendActor(actor);

      // overlay (item, save)
      if(cell.center != MapViewModel::CenterType::Hollow)
      {
        actor.action = getOverlayTile(cell.center);
        actor.zOrder = 12;

        if(cell.center == MapViewModel::CenterType::Player)
          actor.effect = Effect::Blinking;

        presenter->sendActor(actor);
      }
    }
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

  const Room& playerRoom = map.quest->rooms[map.level];
  Vec2i playerPos = playerRoom.pos;
  playerPos.x += int(map.playerPos.x) / CELL_SIZE.x;
  playerPos.y += int(map.playerPos.y) / CELL_SIZE.y;
  r.cells.get(playerPos.x, playerPos.y).center = MapViewModel::CenterType::Player;

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

