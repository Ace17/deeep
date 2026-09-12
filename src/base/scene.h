// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Game, as seen by the outside world

#pragma once

struct Control
{
  // player directions
  char left, right, up, down;

  // player actions
  char menu;
  char start;
  char fire;
  char jump;
  char dash;
  char restart; // kill the player (in case of getting stuck).

  char debug; // toggle debug-mode

  static constexpr char JustPressed = 0x3;
};

struct Scene
{
  virtual ~Scene() = default;

  // advance the scene simulation to the next frame
  // returns the next scene
  virtual Scene* tick(Control c) = 0;

  // ask the scene to send its actors for rendering
  virtual void draw() = 0;
};

struct NullScene : Scene
{
  Scene* tick(Control) override { return this; }
  void draw() override {}
};

extern NullScene nullScene;

