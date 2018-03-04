/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Game, as seen by the outside world

#pragma once
#include <vector>

#include "util.h"
#include "geom.h"

using namespace std;

typedef int SOUND;
typedef int MODEL;

enum class Effect
{
  Normal,
  Blinking,
};

// a game object, as seen by the user-interface, i.e a displayable object.
struct Actor
{
  Actor(Vector2f pos_ = Vector2f(0, 0), MODEL model_ = 0) : pos(pos_), model(model_)
  {
  }

  Vector2f pos = Vector2f(0, 0);
  MODEL model = 0;
  int action = 0;
  float ratio = 0; // in [0 .. 1]
  Size2f scale = Size2f(1, 1);
  Effect effect = Effect::Normal;
  bool useWorldRefFrame = true; // set to 'false' for HUD objects
};

struct Control
{
  bool left, right, up, down;
  bool fire;
  bool jump;
  bool dash;

  bool debug;
};

// outside world, seen by the game
struct View
{
  virtual void textBox(char const* msg) = 0;
  virtual void playMusic(int id) = 0;
  virtual void playSound(int id) = 0;
  virtual void setCameraPos(Vector2f pos) = 0;
  virtual void setAmbientLight(float amount) = 0;
};

// game, seen by the outside world

struct Scene
{
  virtual void tick(Control const& c) = 0;
  virtual vector<Actor> getActors() const = 0;
};

