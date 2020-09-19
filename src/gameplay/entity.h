/*
 * Copyright (C) 2017 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Entity: base game object

#pragma once

#include "base/geom.h"
#include "base/scene.h"
#include "body.h"
#include "game.h"
#include "physics_probe.h"
#include <vector>

using namespace std;

struct Climbable
{
  virtual ~Climbable() = default;
};

struct Damageable
{
  virtual void onDamage(int amount) = 0;
};

struct Entity : Body
{
  virtual ~Entity() = default;

  virtual void enter() {}
  virtual void leave() {}
  virtual void tick() {}

  virtual void addActors(vector<Actor>& actors) const = 0;

  int id = 0;
  bool dead = false;
  int blinking = 0;
  IGame* game = nullptr;
  IPhysicsProbe* physics = nullptr;

  Vector getCenter() const
  {
    return pos + size * 0.5;
  }
};

