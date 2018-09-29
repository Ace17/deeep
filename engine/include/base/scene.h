// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Game, as seen by the outside world

#pragma once

struct Control
{
  // player directions
  bool left, right, up, down;

  // player actions
  bool fire;
  bool jump;
  bool dash;
  bool restart; // kill the player (in case of getting stuck).

  bool debug; // toggle debug-mode
};

// game, seen by the outside world

struct Scene
{
  virtual ~Scene() = default;

  // resets the scene
  virtual void init() {};

  // advance the scene simulation to the next frame
  virtual void tick(Control c) = 0;
};

