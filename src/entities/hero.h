// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include <memory>

struct Player;
std::unique_ptr<Player> makeRockman();

enum ACTION
{
  ACTION_VICTORY,
  ACTION_STAND,
  ACTION_STAND_SHOOT,
  ACTION_BALL,
  ACTION_ENTRANCE,
  ACTION_WALK,
  ACTION_WALK_SHOOT,
  ACTION_DASH,
  ACTION_DASH_AIM,
  ACTION_JUMP,
  ACTION_FALL,
  ACTION_FALL_SHOOT,
  ACTION_LADDER,
  ACTION_LADDER_END,
  ACTION_LADDER_SHOOT,
  ACTION_HADOKEN,
  ACTION_SLIDE,
  ACTION_SLIDE_SHOOT,
  ACTION_CLIMB,
  ACTION_HURT,
  ACTION_FULL,
};

