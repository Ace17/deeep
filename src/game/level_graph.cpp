#include "room.h"
#include "level_common.h"
#include "entities/detector.h"

void addRandomWidgets(Matrix<int>& tiles)
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

Room loadTrainingLevel(IGame* game);

Room loadTinyQuest(IGame* game);
Room loadLevel2(IGame* game);
Room loadLevel3(IGame* game);

static auto const allRooms = makeVector(
{
  &loadTrainingLevel,
  &loadTinyQuest,
  &loadLevel3,
  // &loadLevel2,
});

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

Vector2f toVector2f(Vector2i v)
{
  return Vector2f(v.x, v.y);
}

void addBoundaryDetectors(vector<Room>& quest, int roomIdx, IGame* game)
{
  auto const& room = quest[roomIdx];

  auto const CELL_SIZE = 16;

  auto addDetector =
    [&] ()
    {
      auto detector = make_unique<RoomBoundaryDetector>();
      detector->size = Size2f(1, 1) * CELL_SIZE;
      return detector;
    };

  auto tryToConnectRoom =
    [&] (Vector2i delta, Vector2f margin)
    {
      auto const neighboorPos = room.pos + delta;
      auto const neighboorIdx = getRoomAt(quest, neighboorPos);

      if(neighboorIdx < 0)
        return;

      auto& otherRoom = quest[neighboorIdx];

      auto detector = addDetector();
      detector->pos = toVector2f(delta * CELL_SIZE);
      detector->targetLevel = neighboorIdx;
      detector->transform = toVector2f(room.pos - otherRoom.pos) * CELL_SIZE;
      detector->transform += margin;
      game->spawn(detector.release());
    };

  // left
  for(int row = 0; row < room.size.height; ++row)
  {
    auto const delta = Vector2i(-1, row);
    auto const margin = Vector2f(-1, 0);
    tryToConnectRoom(delta, margin);
  }

  // right
  for(int row = 0; row < room.size.height; ++row)
  {
    auto const delta = Vector2i(room.size.width, row);
    auto const margin = Vector2f(1, 0);
    tryToConnectRoom(delta, margin);
  }

  // bottom
  for(int col = 0; col < room.size.width; ++col)
  {
    auto const delta = Vector2i(col, -1);
    auto const margin = Vector2f(0, -1);
    tryToConnectRoom(delta, margin);
  }
}

Room Graph_loadRoom(int roomIdx, IGame* game)
{
  extern vector<Room> loadQuest(string path);
  auto quest = loadQuest("res/quest.json");

  Room r;

  if(roomIdx >= 13)
  {
    roomIdx -= 13;
    roomIdx = clamp<int>(roomIdx, 0, allRooms.size() - 1);
    r = allRooms[roomIdx] (game);
  }
  else
  {
    if(roomIdx < 0 || roomIdx >= (int)quest.size())
      throw runtime_error("No such level");

    r = move(quest[roomIdx]);

    addBoundaryDetectors(quest, roomIdx, game);
  }

  addRandomWidgets(r.tiles);

  if(r.name == "")
    r.name = "unknown room";

  return r;
}

