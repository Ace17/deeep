#pragma once

#include "game/entity.h"

struct Player : Entity
{
  virtual void think(Control const& s) = 0;
  virtual float health() = 0;
  virtual void addUpgrade(Int upgrade) = 0;
};

enum
{
  UPGRADE_SHOOT = 1,
  UPGRADE_CLIMB = 2,
  UPGRADE_DASH = 4,
};

Player* createRockman();

