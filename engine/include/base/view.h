// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Outside world, as seen by the game

#pragma once

#include "geom.h"
#include "resource.h"

typedef int SOUND;
typedef int MUSIC;
typedef int MODEL;

enum class Effect
{
  Normal,
  Blinking,
};

// a displayable object (= a game object, as seen by the user-interface)
struct Actor
{
  Vector2f pos = Vector2f(0, 0); // object position, in logical units
  MODEL model = 0; // what sprite to display
  int action = 0; // what sprite action to use
  float ratio = 0; // in [0 .. 1]. 0 for action beginning, 1 for action end
  float angle = 0;
  Size2f scale = Size2f(1, 1); // sprite size
  Effect effect = Effect::Normal;
  bool screenRefFrame = false; // if true, 'pos' is expressed relative to the camera (used for HUD objects).
  int zOrder = 0; // actors with higher value are drawn over the others
};

// This interface should act as a message sink.
// It should provide no way to query anything about the outside world.
struct View
{
  virtual ~View() = default;

  virtual void setTitle(char const* gameTitle) = 0;
  virtual void preload(Resource res) = 0;
  virtual void textBox(char const* msg) = 0;
  virtual void playMusic(MUSIC id) = 0;
  virtual void stopMusic() = 0;
  virtual void playSound(SOUND id) = 0;
  virtual void setCameraPos(Vector2f pos) = 0;
  virtual void setAmbientLight(float amount) = 0;

  // adds a displayable object to the current frame
  virtual void sendActor(Actor const& actor) = 0;
};

