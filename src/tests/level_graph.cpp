/*
 * Copyright (C) 2021 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "base/geom.h"
#include "gameplay/quest.h"
#include "tests.h"
#include <vector>

int getRoomAt(std::vector<Room> const& quest, Vec2i absPos);

unittest("LevelGraph: getRoomAt: no rooms")
{
  std::vector<Room> rooms;
  assertEquals(-1, getRoomAt(rooms, Vec2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: one zero sized room")
{
  std::vector<Room> rooms;
  Room room;
  room.pos = Vec2i(0, 0);
  room.size = Vec2i(0, 0);
  rooms.push_back(std::move(room));
  assertEquals(-1, getRoomAt(rooms, Vec2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: 1x1 room at (0;0)")
{
  std::vector<Room> rooms;
  Room room;
  room.pos = Vec2i(0, 0);
  room.size = Vec2i(1, 1);
  rooms.push_back(std::move(room));
  assertEquals(0, getRoomAt(rooms, Vec2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: 1x1 room at (0;0), 1x1 room at (1;0)")
{
  std::vector<Room> rooms;
  {
    Room room;
    room.pos = Vec2i(0, 0);
    room.size = Vec2i(1, 1);
    rooms.push_back(std::move(room));
  }
  {
    Room room;
    room.pos = Vec2i(1, 0);
    room.size = Vec2i(1, 1);
    rooms.push_back(std::move(room));
  }
  assertEquals(1, getRoomAt(rooms, Vec2i(1, 0)));
  assertEquals(-1, getRoomAt(rooms, Vec2i(2, 0)));
}

unittest("LevelGraph: getRoomAt: 1x2 room at (10;4)")
{
  std::vector<Room> rooms;
  {
    Room room;
    room.pos = Vec2i(10, 4);
    room.size = Vec2i(1, 2);
    rooms.push_back(std::move(room));
  }
  assertEquals(-1, getRoomAt(rooms, Vec2i(10, 3)));
  assertEquals(0, getRoomAt(rooms, Vec2i(10, 4)));
  assertEquals(0, getRoomAt(rooms, Vec2i(10, 5)));
  assertEquals(-1, getRoomAt(rooms, Vec2i(10, 6)));

  assertEquals(-1, getRoomAt(rooms, Vec2i(9, 4)));
}

