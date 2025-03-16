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
#include <climits>
#include <cstdlib> // rand

static
bool isInsideRoom(Vec2i pos, Room const& room)
{
  if(pos.x < room.pos.x)
    return false;

  if(pos.x >= room.pos.x + room.size.x)
    return false;

  if(pos.y < room.pos.y)
    return false;

  if(pos.y >= room.pos.y + room.size.y)
    return false;

  return true;
}

int getRoomAt(std::vector<Room> const& quest, Vec2i absPos)
{
  for(int i = 0; i < (int)quest.size(); ++i)
  {
    if(isInsideRoom(absPos, quest[i]))
      return i;
  }

  return -1;
}

extern const Vec2i CELL_SIZE;

static
Vector toTilePosition(Vec2i v)
{
  return Vector(v.x * CELL_SIZE.x, v.y * CELL_SIZE.y);
}

static Vec2i operator * (Vec2i a, Vec2i b)
{
  return { a.x * b.x, a.y * b.y };
}

static
void addBoundaryDetectors(Room& room, std::vector<Room> const& quest)
{
  auto tryToConnectRoom = [&] (Vec2i delta, Vec2i margin)
    {
      auto const neighboorPos = room.pos + delta;
      auto const neighboorIdx = getRoomAt(quest, neighboorPos);

      if(neighboorIdx < 0)
      {
        auto pos = toTilePosition(delta);
        room.spawners.push_back({ 0, pos, "blocker" });
        return;
      }

      auto& otherRoom = quest[neighboorIdx];

      auto transform = (room.pos - otherRoom.pos) * CELL_SIZE + margin;

      Room::Spawner s {};
      s.name = "room_boundary_detector";
      s.config["target_level"] = std::to_string(neighboorIdx);
      s.config["transform_x"] = std::to_string(transform.x);
      s.config["transform_y"] = std::to_string(transform.y);
      s.pos = toTilePosition(delta);
      room.spawners.push_back(s);
    };

  // left
  for(int row = 0; row < room.size.y; ++row)
  {
    auto const delta = Vec2i(-1, row);
    auto const margin = Vec2i(-0.1, 0);
    tryToConnectRoom(delta, margin);
  }

  // right
  for(int row = 0; row < room.size.y; ++row)
  {
    auto const delta = Vec2i(room.size.x, row);
    auto const margin = Vec2i(+0.1, 0);
    tryToConnectRoom(delta, margin);
  }

  // bottom
  for(int col = 0; col < room.size.x; ++col)
  {
    auto const delta = Vec2i(col, -1);
    auto const margin = Vec2i(0, -0.1);
    tryToConnectRoom(delta, margin);
  }

  // top
  for(int col = 0; col < room.size.x; ++col)
  {
    auto const delta = Vec2i(col, room.size.y);
    auto const margin = Vec2i(0, 0.1);
    tryToConnectRoom(delta, margin);
  }
}

static
std::vector<std::string> parseCall(std::string content)
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
        throw Error(std::string("Expected '") + what + "'");
    };

  auto parseString = [&] ()
    {
      std::string r;

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
      std::string r;

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

  std::vector<std::string> r;
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
      spawner.config[std::to_string(i++)] = varValue;
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
      spawner.config["tile"] = std::to_string(tile);
      spawner.config["theme"] = std::to_string(room.theme);
      room.tiles.set(spawner.pos.x, spawner.pos.y, 0);
      room.tilesForDisplay.set(spawner.pos.x, spawner.pos.y, -1);
    }
  }
}

static
void preprocessRoom(Room& room, std::vector<Room> const& quest)
{
  addBoundaryDetectors(room, quest);
  replaceLegacyEntityNames(room);
  addFragileBlocks(room);
}

void preprocessQuest(Quest& quest)
{
  for(auto& r : quest.rooms)
    preprocessRoom(r, quest.rooms);
}

