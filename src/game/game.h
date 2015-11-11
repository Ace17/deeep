/**
 * @file game.h
 * @brief Game, as seen by entities
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

#include "engine/geom.h"
#include "engine/scene.h"

class Entity;

struct ITriggerable
{
  virtual void trigger() = 0;
};

struct IGame
{
  virtual void playSound(SOUND id) = 0;
  virtual void spawn(Entity* e) = 0;
  virtual bool isSolid(Vector2f pos) = 0;
  virtual void trigger(int triggerIdx) = 0;
  virtual void listen(int triggerIdx, ITriggerable*) = 0;
};

