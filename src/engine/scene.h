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

#include "geom.h"

using namespace std;

enum MODEL_TYPE
{
  MODEL_BASE,
  NUM_MODELS,
};

enum SOUND_TYPE
{
  SOUND_BASE,
  NUM_SOUNDS,
};

enum EFFECT_TYPE
{
  EFFECT_NORMAL,
  EFFECT_BLINKING,
};

struct Actor
{
  Actor(Vector2f pos_ = Vector2f(0, 0), MODEL_TYPE model_ = MODEL_BASE) : pos(pos_), model(model_)
  {
    scale = Vector2f(1, 1);
    effect = EFFECT_NORMAL;
    frame = 0;
  }

  Vector2f pos;
  MODEL_TYPE model;
  int frame;
  Vector2f scale;
  EFFECT_TYPE effect;
};

struct Control
{
  bool left, right, up, down;
  bool fire;
};

// game, seen by the outside world

struct Scene
{
  virtual void tick(Control const& c) = 0;
  virtual vector<Actor> getActors() const = 0;
  virtual vector<SOUND_TYPE> readSounds() = 0;
};

