#include "level_graph.h"
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

Level loadTrainingLevel(IGame* game);

Level loadTinyQuest(IGame* game);
Level loadLevel2(IGame* game);
Level loadLevel3(IGame* game);

static auto const allLevels = makeVector(
{
  &loadTrainingLevel,
  &loadTinyQuest,
  &loadLevel3,
  // &loadLevel2,
});

bool isInsideRoom(Vector2i pos, Level const& room)
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

int getRoomAt(vector<Level> const& quest, Vector2i absPos)
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

void addBoundaryDetectors(vector<Level>& quest, int roomIdx, IGame* game)
{
  auto const& room = quest[roomIdx];

  auto const CELL_SIZE = 16;

  // right
  for(int row = 0; row < room.size.height; ++row)
  {
    auto const neighboorPos = room.pos + Vector2i(room.size.width, row);
    auto const neighboorIdx = getRoomAt(quest, neighboorPos);

    if(neighboorIdx < 0)
      continue;

    auto& otherRoom = quest[neighboorIdx];

    auto detector = make_unique<LevelBoundaryDetector>();
    detector->size = Size2f(1, 1) * CELL_SIZE;
    detector->pos = Vector2f(room.size.width, row) * CELL_SIZE;
    detector->targetLevel = neighboorIdx;
    detector->transform = toVector2f(room.pos - otherRoom.pos) * CELL_SIZE;
    detector->transform += Vector2f(1, 0); // margin
    game->spawn(detector.release());
  }

  // left
  for(int row = 0; row < room.size.height; ++row)
  {
    auto const neighboorPos = room.pos + Vector2i(-1, row);
    auto const neighboorIdx = getRoomAt(quest, neighboorPos);

    if(neighboorIdx < 0)
      continue;

    auto& otherRoom = quest[neighboorIdx];

    auto detector = make_unique<LevelBoundaryDetector>();
    detector->size = Size2f(1, 1) * CELL_SIZE;
    detector->pos = Vector2f(-1, row) * CELL_SIZE;
    detector->targetLevel = neighboorIdx;
    detector->transform = toVector2f(room.pos - otherRoom.pos) * CELL_SIZE;
    detector->transform += Vector2f(-1, 0); // margin
    game->spawn(detector.release());
  }
}

Level Graph_loadLevel(int levelIdx, IGame* game)
{
  extern vector<Level> loadQuest(string path);
  auto quest = loadQuest("res/quest.json");

  Level r;

  if(levelIdx >= 13)
  {
    levelIdx -= 13;
    levelIdx = clamp<int>(levelIdx, 0, allLevels.size() - 1);
    r = allLevels[levelIdx] (game);
  }
  else
  {
    if(levelIdx < 0 || levelIdx >= (int)quest.size())
      throw runtime_error("No such level");

    r = move(quest[levelIdx]);

    addBoundaryDetectors(quest, levelIdx, game);
  }

  addRandomWidgets(r.tiles);
  return r;
}

