#pragma once

#include "base/box.h"
#include <stdint.h> // uintptr_t
#include <vector>

struct HashedSpace
{
public:
  HashedSpace();

  void putObject(Rect2f where, uintptr_t what);
  void removeObject(Rect2f where, uintptr_t what);

  std::vector<uintptr_t> getObjectsInRect(Rect2f where) const;

private:
  Vec2i getVirtualCellCoords(Vec2f pos) const;

  struct Object
  {
    Rect2f where;
    uintptr_t data;
  };

  struct Cell
  {
    std::vector<Object> objects;
  };

  std::vector<Cell> m_cells;
};

