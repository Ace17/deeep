/**
 * @brief Game, as seen by the outside world
 * @author Sebastien Alaiwan
 */

/*
 * Copyright (C) 2015 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once
#include <vector>

#include "util.h"
#include "geom.h"

using namespace std;

typedef int SOUND;
typedef int MODEL;

enum EFFECT_TYPE
{
  EFFECT_NORMAL,
  EFFECT_BLINKING,
};

struct Actor
{
  Actor(Vector2f pos_ = Vector2f(0, 0), MODEL model_ = 0) : pos(pos_), model(model_)
  {
    scale = Vector2f(1, 1);
    effect = EFFECT_NORMAL;
    ratio = 0;
  }

  Vector2f pos;
  MODEL model;
  int action = 0;
  float ratio = 1; // in [0 .. 1]
  Vector2f scale;
  EFFECT_TYPE effect = EFFECT_NORMAL;
};

struct Control
{
  bool left, right, up, down;
  bool fire;
  bool jump;
  bool dash;
};

struct Resource
{
  int id;
  char const* path;
};

// game, seen by the outside world

struct Scene
{
  virtual Span<const Resource> getSounds() const = 0;
  virtual Span<const Resource> getModels() const = 0;

  virtual void tick(Control const& c) = 0;
  virtual vector<Actor> getActors() const = 0;
  virtual vector<SOUND> readSounds() = 0;
};

