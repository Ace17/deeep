// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Game, as seen by the outside world

#pragma once
#include <vector>

#include "util.h"
#include "geom.h"

using namespace std;

typedef int MODEL;

enum class Effect
{
  Normal,
  Blinking,
};

// a displayable object (= a game object, as seen by the user-interface)
struct Actor
{
  Vector2f pos = Vector2f(0, 0); // object position, in logical units
  MODEL model = 0; // what sprite to display
  int action = 0; // what sprite action to use
  float ratio = 0; // in [0 .. 1]. 0 for action beginning, 1 for action end
  Size2f scale = Size2f(1, 1); // sprite size
  Effect effect = Effect::Normal;
  bool screenRefFrame = false; // if true, 'pos' is expressed relative to the camera (used for HUD objects).
  int zOrder = 0; // actors with higher value are drawn over the others
};

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

  // return a list of displayable objects for the current frame
  virtual vector<Actor> getActors() const = 0;
};

