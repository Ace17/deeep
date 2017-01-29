#include "level_graph.h"

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

void loadTrainingLevel(Matrix<int>& tiles, Vector2i& start, IGame* game);

void loadTinyQuest(Matrix<int>& tiles, Vector2i& start, IGame* game);
void loadLevel2(Matrix<int>& tiles, Vector2i& start, IGame* game);
void loadLevel3(Matrix<int>& tiles, Vector2i& start, IGame* game);

static auto const allLevels = makeVector(
{
  &loadTrainingLevel,
  &loadTinyQuest,
  &loadLevel3,
  // &loadLevel2,
});

void Graph_loadLevel(int levelIdx, Matrix<int>& tiles, IGame* game, Vector2i& start)
{
  levelIdx = clamp<int>(levelIdx, 0, allLevels.size() - 1);
  allLevels[levelIdx] (tiles, start, game);

  addRandomWidgets(tiles);
}

