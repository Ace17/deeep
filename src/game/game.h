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

#include "base/geom.h"
#include "base/scene.h"

struct Entity;

struct Event
{
  virtual ~Event()
  {
  }; // force a vtable so we can dynamic_cast events

  template<typename T>
  const T* as() const
  {
    return dynamic_cast<const T*>(this);
  }
};

struct EndLevelEvent : Event
{
};

struct IEventSink
{
  virtual void notify(const Event* evt) = 0;
};

struct IGame
{
  virtual void playSound(SOUND id) = 0;
  virtual void spawn(Entity* e) = 0;
  virtual bool isSolid(Vector2f pos) = 0;
  virtual void postEvent(unique_ptr<Event> event) = 0;
  virtual void subscribeForEvents(IEventSink*) = 0;
  virtual Vector2f getPlayerPosition() = 0;
};

