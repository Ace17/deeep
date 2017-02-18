#include <vector>
#include "tests/tests.h"
#include "base/geom.h"
#include "game/level_graph.h"
using namespace std;

int getRoomAt(vector<Level> const& quest, Vector2i absPos);

unittest("LevelGraph: getRoomAt: no rooms")
{
  vector<Level> rooms;
  assertEquals(-1, getRoomAt(rooms, Vector2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: one zero sized room")
{
  vector<Level> rooms;
  Level room;
  room.pos = Vector2i(0, 0);
  room.size = Size2i(0, 0);
  rooms.push_back(move(room));
  assertEquals(-1, getRoomAt(rooms, Vector2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: 1x1 room at (0;0)")
{
  vector<Level> rooms;
  Level room;
  room.pos = Vector2i(0, 0);
  room.size = Size2i(1, 1);
  rooms.push_back(move(room));
  assertEquals(0, getRoomAt(rooms, Vector2i(0, 0)));
}

unittest("LevelGraph: getRoomAt: 1x1 room at (0;0), 1x1 room at (1;0)")
{
  vector<Level> rooms;
  {
    Level room;
    room.pos = Vector2i(0, 0);
    room.size = Size2i(1, 1);
    rooms.push_back(move(room));
  }
  {
    Level room;
    room.pos = Vector2i(1, 0);
    room.size = Size2i(1, 1);
    rooms.push_back(move(room));
  }
  assertEquals(1, getRoomAt(rooms, Vector2i(1, 0)));
  assertEquals(-1, getRoomAt(rooms, Vector2i(2, 0)));
}

unittest("LevelGraph: getRoomAt: 1x2 room at (10;4)")
{
  vector<Level> rooms;
  {
    Level room;
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

