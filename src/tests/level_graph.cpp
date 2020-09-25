/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "base/geom.h"
#include "gameplay/quest.h"
#include "tests.h"
#include <vector>
using namespace std;

int getRoomAt(vector<Room> const& quest, Vector2i absPos);

unittest("LevelGraph: getRoomAt: no rooms")
{
  vector<Room> rooms;
  assertEquals(-1, getRoomAt(rooms, Vector2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: one zero sized room")
{
  vector<Room> rooms;
  Room room;
  room.pos = Vector2i(0, 0);
  room.size = Size2i(0, 0);
  rooms.push_back(move(room));
  assertEquals(-1, getRoomAt(rooms, Vector2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: 1x1 room at (0;0)")
{
  vector<Room> rooms;
  Room room;
  room.pos = Vector2i(0, 0);
  room.size = Size2i(1, 1);
  rooms.push_back(move(room));
  assertEquals(0, getRoomAt(rooms, Vector2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: 1x1 room at (0;0), 1x1 room at (1;0)")
{
  vector<Room> rooms;
  {
    Room room;
    room.pos = Vector2i(0, 0);
    room.size = Size2i(1, 1);
    rooms.push_back(move(room));
  }
  {
    Room room;
    room.pos = Vector2i(1, 0);
    room.size = Size2i(1, 1);
    rooms.push_back(move(room));
  }
  assertEquals(1, getRoomAt(rooms, Vector2i(1, 0)));
  assertEquals(-1, getRoomAt(rooms, Vector2i(2, 0)));
}

unittest("LevelGraph: getRoomAt: 1x2 room at (10;4)")
{
  vector<Room> rooms;
  {
    Room room;
    room.pos = Vector2i(10, 4);
    room.size = Size2i(1, 2);
    rooms.push_back(move(room));
  }
  assertEquals(-1, getRoomAt(rooms, Vector2i(10, 3)));
  assertEquals(0, getRoomAt(rooms, Vector2i(10, 4)));
  assertEquals(0, getRoomAt(rooms, Vector2i(10, 5)));
  assertEquals(-1, getRoomAt(rooms, Vector2i(10, 6)));

  assertEquals(-1, getRoomAt(rooms, Vector2i(9, 4)));
}

