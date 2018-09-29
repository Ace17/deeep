/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Entity: base game object

#pragma once

#include <vector>
#include "base/scene.h"
#include "base/geom.h"
#include "game.h"
#include "body.h"
#include "physics_probe.h"

using namespace std;

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

  bool dead = false;
  int blinking = 0;
  IGame* game = nullptr;
  IPhysicsProbe* physics = nullptr;

  Vector getCenter() const
  {
    return pos + size * 0.5;
  }
};

