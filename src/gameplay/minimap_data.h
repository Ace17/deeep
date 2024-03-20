#pragma once
#include "base/geom.h"
#include "base/matrix.h"

struct Quest;

struct MinimapData
{
  const Quest* quest;
  int level;
  Vec2f playerPos;
};

///////////////////////////////////////////////////////////////////////////////
// The map, as the player sees it.
// This has a one-to-one correspondance with what's visible on screen.
struct MapViewModel
{
  enum class CenterType
  {
    Solid, // cell is not part of any room
    Hollow, // cell is part of a room
    Item,
    Save,
    Player,
  };

  enum class EdgeType
  {
    Wall, // cells are separated by a wall
    Free, // cells are connected and belong to the same room
    Door, // cells are connected and belong to two different rooms
  };

  struct Cell
  {
    bool visited;
    CenterType center;
    EdgeType up;
    EdgeType right;
  };

  Matrix2<Cell> cells;
};

MapViewModel computeMapViewModel(const MinimapData& map);

struct IPresenter;
void drawMinimap(IPresenter* presenter, Vec2f pos, const MapViewModel& vm);

