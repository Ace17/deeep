#pragma once

#include "game/entity.h"

class Player : public Entity
{
public:
  virtual void think(Control const& s) = 0;
  virtual float health() = 0;
};

Player* createRockman();

