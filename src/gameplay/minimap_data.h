#pragma once
#include "base/geom.h"

struct Quest;

struct MinimapData
{
  const Quest* quest;
  int level;
  Vec2f playerPos;
};

