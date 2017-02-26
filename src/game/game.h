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

struct TouchLevelBoundary : Event
{
  TouchLevelBoundary(int targetLevel_, Vector2f transform_)
  {
    targetLevel = targetLevel_;
    transform = transform_;
  }

  int targetLevel;
  Vector2f transform {};
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

struct IGame
{
  virtual void playSound(SOUND id) = 0;
  virtual void spawn(Entity* e) = 0;
  virtual bool isSolid(Vector2f pos) = 0;
  virtual void postEvent(unique_ptr<Event> event) = 0;
  virtual void subscribeForEvents(IEventSink*) = 0;
  virtual Vector2f getPlayerPosition() = 0;

  bool isSolid(Vector2f pos, Size2f size)
  {
    if(isSolid(pos + Vector2f(0, 0)))
      return true;

    if(isSolid(pos + Vector2f(size.width, 0)))
      return true;

    if(isSolid(pos + Vector2f(0, size.height)))
      return true;

    if(isSolid(pos + Vector2f(size.width, size.height)))
      return true;

    return false;
  }
};

