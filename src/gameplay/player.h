// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "entity.h"

struct Player
{
  virtual ~Player() = default;

  virtual void think(Control const& s) = 0;
  virtual float health() = 0;
  virtual Vector position() = 0;
  virtual void setPosition(Vector) = 0;
  virtual void addUpgrade(int upgrade) = 0;
  virtual void enterLevel() {};
  virtual void leaveLevel() {};
};

enum
{
  UPGRADE_SHOOT = 1,
  UPGRADE_CLIMB = 2,
  UPGRADE_DASH = 4,
  UPGRADE_DJUMP = 8,
  UPGRADE_BALL = 16,
  UPGRADE_SLIDE = 32,
  UPGRADE_BOMB = 64,
};

struct IGame;
Player* createHeroPlayer(IGame* game);
Player* createHumanPlayer(IGame* game);

