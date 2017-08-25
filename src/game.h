/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

// Game, as seen by entities

#pragma once

#include <functional>
#include "base/geom.h"
#include "base/scene.h"

typedef Matrix2<int> Matrix;
typedef Vector2f Vector;

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

struct TouchLevelBoundary : Event
{
  TouchLevelBoundary(int targetLevel_, Vector transform_)
  {
    targetLevel = targetLevel_;
    transform = transform_;
  }

  int targetLevel;
  Vector transform {};
};

struct IEventSink
{
  virtual void notify(const Event* evt) = 0;
};

struct EventDelegator : IEventSink
{
  void notify(const Event* evt)
  {
    m_onNotify(evt);
  }

  function<void(const Event*)> m_onNotify;
};

template<typename EventType>
EventDelegator makeDelegator(function<void(const EventType*)> handler)
{
  EventDelegator r;
  r.m_onNotify = [ = ] (const Event* evt)
    {
      if(auto event = evt->as<EventType>())
        handler(event);
    };
  return r;
}

struct Handle
{
  virtual ~Handle() {};
};

struct IGame
{
  virtual void playSound(SOUND id) = 0;
  virtual void spawn(Entity* e) = 0;
  virtual void postEvent(unique_ptr<Event> event) = 0;
  virtual unique_ptr<Handle> subscribeForEvents(IEventSink*) = 0;
  virtual Vector getPlayerPosition() = 0;
  virtual void textBox(char const* msg) = 0;
  virtual void setAmbientLight(float light) = 0;
};

