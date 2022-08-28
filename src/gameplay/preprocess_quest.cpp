// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Preprocessor for rooms:
// - create connections between adjacent rooms
// - create blockers around rooms
// - add some display randomness to the tiles

#include "base/error.h"
#include "quest.h"
#include <cstdlib> // rand

static
void addRandomWidgets(Matrix2<int>& tiles)
{
  auto rect = [&] (Vector2i pos, Size2i size, int tile)
    {
      for(int dy = 0; dy < size.height; ++dy)
        for(int dx = 0; dx < size.width; ++dx)
          tiles.set(dx + pos.x, dy + pos.y, tile);
    };

  auto isFull = [&] (Vector2i pos, Size2i size)
    {
      for(int dy = 0; dy < size.height; ++dy)
        for(int dx = 0; dx < size.width; ++dx)
          if(tiles.get(dx + pos.x, dy + pos.y) == 0)
            return false;

      return true;
    };

  auto const maxX = tiles.size.width - 4;
  auto const maxY = tiles.size.height - 4;

  for(int i = 0; i < (maxX * maxY) / 100; ++i)
  {
    auto pos = Vector2i(rand() % maxX + 1, rand() % maxY + 1);
    auto size = Size2i(2, 2);

    if(isFull(pos + Vector2i(-1, -1), Size2i(size.width + 2, size.height + 2)))
      rect(pos, size, 3);
  }
}

static
bool isInsideRoom(Vector2i pos, Room const& room)
{
  if(pos.x < room.pos.x)
    return false;

  if(pos.x >= room.pos.x + room.size.width)
    return false;

  if(pos.y < room.pos.y)
    return false;

  if(pos.y >= room.pos.y + room.size.height)
    return false;

  return true;
}

int getRoomAt(vector<Room> const& quest, Vector2i absPos)
{
  for(int i = 0; i < (int)quest.size(); ++i)
  {
    if(isInsideRoom(absPos, quest[i]))
      return i;
  }

  return -1;
}

extern const Size2i CELL_SIZE;

static
Vector toTilePosition(Vector2i v)
{
  return Vector(v.x * CELL_SIZE.width, v.y * CELL_SIZE.height);
}

static Vector2i operator * (Vector2i a, Size2i b)
{
  return { a.x * b.width, a.y * b.height };
}

static
void addBoundaryDetectors(Room& room, vector<Room> const& quest)
{
  auto tryToConnectRoom = [&] (Vector2i delta, Vector2i margin)
    {
      auto const neighboorPos = room.pos + delta;
      auto const neighboorIdx = getRoomAt(quest, neighboorPos);

      if(neighboorIdx < 0)
      {
        auto pos = toTilePosition(delta);
        room.spawners.push_back({ pos, "blocker" });
        return;
      }

      auto& otherRoom = quest[neighboorIdx];

      auto transform = (room.pos - otherRoom.pos) * CELL_SIZE + margin;

      Room::Spawner s;
      s.name = "room_boundary_detector";
      s.config["target_level"] = to_string(neighboorIdx);
      s.config["transform_x"] = to_string(transform.x);
      s.config["transform_y"] = to_string(transform.y);
      s.pos = toTilePosition(delta);
      room.spawners.push_back(s);
    };

  // left
  for(int row = 0; row < room.size.height; ++row)
  {
    auto const delta = Vector2i(-1, row);
    auto const margin = Vector2i(-1, 0);
    tryToConnectRoom(delta, margin);
  }

  // right
  for(int row = 0; row < room.size.height; ++row)
  {
    auto const delta = Vector2i(room.size.width, row);
    auto const margin = Vector2i(1, 0);
    tryToConnectRoom(delta, margin);
  }

  // bottom
  for(int col = 0; col < room.size.width; ++col)
  {
    auto const delta = Vector2i(col, -1);
    auto const margin = Vector2i(0, -2);
    tryToConnectRoom(delta, margin);
  }

  // top
  for(int col = 0; col < room.size.width; ++col)
  {
    auto const delta = Vector2i(col, room.size.height);
    auto const margin = Vector2i(0, 2);
    tryToConnectRoom(delta, margin);
  }
}

// from smarttiles
int computeTileFor(Matrix2<int> const& m, int x, int y);

static
Matrix2<int> applySmartTiling(Matrix2<int> const& tiles)
{
  Matrix2<int> r;
  r.resize(tiles.size);

  for(int row = 0; row < r.size.height; ++row)
    for(int col = 0; col < r.size.width; ++col)
      r.set(col, row, -1);

  for(int row = 0; row < tiles.size.height; ++row)
  {
    for(int col = 0; col < tiles.size.width; ++col)
    {
      if(tiles.get(col, row) == 0)
        continue;

      auto composition = computeTileFor(tiles, col, row);
      r.set(col, row, composition);
    }
  }

  return r;
}

static
vector<string> parseCall(string content)
{
  content += '\0';
  auto stream = content.c_str();

  auto head = [&] ()
    {
      return *stream;
    };

  auto accept = [&] (char what)
    {
      if(!*stream)
        return false;

      if(head() != what)
        return false;

      stream++;
      return true;
    };

  auto expect = [&] (char what)
    {
      if(!accept(what))
        throw Error(string("Expected '") + what + "'");
    };

  auto parseString = [&] ()
    {
      string r;

      while(!accept('"'))
      {
        char c = head();
        accept(c);
        r += c;
      }

      return r;
    };

  auto parseIdentifier = [&] ()
    {
      string r;

      while(isalnum(head()) || head() == '_' || head() == '-')
      {
        char c = head();
        accept(c);
        r += c;
      }

      return r;
    };

  auto parseArgument = [&] ()
    {
      if(accept('"'))
        return parseString();
      else
        return parseIdentifier();
    };

  vector<string> r;
  r.push_back(parseIdentifier());

  if(accept('('))
  {
    bool first = true;

    while(!accept(')'))
    {
      if(!first)
        expect(',');

      r.push_back(parseArgument());
      first = false;
    }
  }

  return r;
}

static
void replaceLegacyEntityNames(Room& room)
{
  for(auto& spawner : room.spawners)
  {
    auto words = parseCall(spawner.name);
    spawner.name = words[0];
    words.erase(words.begin());

    int i = 0;

    for(auto& varValue : words)
      spawner.config[to_string(i++)] = varValue;
  }
}

static
void addFragileBlocks(Room& room)
{
  for(auto& spawner : room.spawners)
  {
    if(spawner.name != "fragile_block" && spawner.name != "crumble_block")
      continue;

    if(!room.tiles.isInside(spawner.pos.x, spawner.pos.y))
      continue;

    if(room.tiles.get(spawner.pos.x, spawner.pos.y))
    {
      int tile = room.tilesForDisplay.get(spawner.pos.x, spawner.pos.y);
      spawner.config["tile"] = to_string(tile);
      spawner.config["theme"] = to_string(room.theme);
      room.tiles.set(spawner.pos.x, spawner.pos.y, 0);
      room.tilesForDisplay.set(spawner.pos.x, spawner.pos.y, -1);
    }
  }
}

static
void preprocessRoom(Room& room, vector<Room> const& quest)
{
  room.tilesForDisplay = applySmartTiling(room.tiles);

  addRandomWidgets(room.tiles);
  addBoundaryDetectors(room, quest);
  replaceLegacyEntityNames(room);
  addFragileBlocks(room);
}

void preprocessQuest(Quest& quest)
{
  for(auto& r : quest.rooms)
    preprocessRoom(r, quest.rooms);
}

