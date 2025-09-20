#include "base/box.h"
#include "base/util.h" // unstableRemove
#include <algorithm>   // find
#include <math.h>      // floor
#include <stdint.h>    // uintptr_t
#include <vector>

#include "spatial_hashing.h"

namespace
{
constexpr float HashCellSize = 8.0f;
constexpr int BucketCount = 256;

int hash(Vec2i pos)
{
  return std::abs((pos.x * 92837111) ^ (pos.y * 689287499)) % BucketCount;
}
} // namespace

HashedSpace::HashedSpace() : m_cells(BucketCount) {}

void HashedSpace::putObject(Rect2f where, uintptr_t what)
{
  Vec2i min = getVirtualCellCoords(where.pos);
  Vec2i max = getVirtualCellCoords(where.pos + where.size);

  for(int y = min.y; y <= max.y; ++y)
  {
    for(int x = min.x; x <= max.x; ++x)
    {
      const int h = hash({ x, y });
      m_cells[h].objects.push_back(Object{ where, what });
    }
  }
}

void HashedSpace::removeObject(Rect2f where, uintptr_t what)
{
  Vec2i min = getVirtualCellCoords(where.pos);
  Vec2i max = getVirtualCellCoords(where.pos + where.size);

  for(int y = min.y; y <= max.y; ++y)
  {
    for(int x = min.x; x <= max.x; ++x)
    {
      const int h = hash({ x, y });
      auto isItTheOne = [&](const Object& o) { return o.data == what; };
      unstableRemove(m_cells[h].objects, isItTheOne);
    }
  }
}

std::vector<uintptr_t> HashedSpace::getObjectsInRect(Rect2f where) const
{
  std::vector<uintptr_t> result;

  Vec2i min = getVirtualCellCoords(where.pos);
  Vec2i max = getVirtualCellCoords(where.pos + where.size);

  for(int y = min.y; y <= max.y; ++y)
  {
    for(int x = min.x; x <= max.x; ++x)
    {
      const int h = hash({ x, y });

      for(auto& obj : m_cells[h].objects)
      {
        if(overlaps(obj.where, where))
        {
          // the 'result' array has very few elements (less than 10 on
          // average)
          if(std::find(result.begin(), result.end(), obj.data) == result.end())
            result.push_back(obj.data);
        }
      }
    }
  }

  return result;
}

Vec2i HashedSpace::getVirtualCellCoords(Vec2f pos) const
{
  Vec2i r;
  r.x = int(floor(pos.x / HashCellSize));
  r.y = int(floor(pos.y / HashCellSize));
  return r;
}

