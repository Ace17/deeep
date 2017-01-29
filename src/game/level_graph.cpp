#include "level_graph.h"
#include "level_common.h"

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

Level Graph_loadLevel(int levelIdx, IGame* game)
{
  if(levelIdx >= 10)
  {
    levelIdx -= 10;
    extern vector<Level> loadQuest(string path);
    auto quest = loadQuest("res/quest.json");

    if(levelIdx < 0 || levelIdx >= (int)quest.size())
      throw runtime_error("No such level");

    return move(quest[levelIdx]);
  }

  levelIdx = clamp<int>(levelIdx, 0, allLevels.size() - 1);
  auto r = allLevels[levelIdx] (game);
  addRandomWidgets(r.tiles);
  return r;
}

