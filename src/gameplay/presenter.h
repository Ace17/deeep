// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Outside world, as seen by the game

#pragma once

#include "base/geom.h"
#include "base/resource.h"

typedef int SOUND;
typedef int MUSIC;
typedef int MODEL;

enum class Effect
{
  Normal,
  Blinking,
};

// a displayable object (= a game object, as seen by the user-interface)
struct SpriteActor
{
  Vec2f pos = Vec2f(0, 0); // object center position, in logical units
  MODEL model = 0; // what sprite to display
  int action = 0; // what sprite action to use
  float ratio = 0; // in [0 .. 1]. 0 for action beginning, 1 for action end
  float angle = 0;
  Vec2f scale = { 1, 1 }; // sprite size
  Effect effect = Effect::Normal;
  bool screenRefFrame = false; // if true, 'pos' is expressed relative to the camera (used for HUD objects).
  int zOrder = 0; // actors with higher value are drawn over the others
};

struct DebugRectActor
{
  Vec2f pos[2];
};

// This interface should act as a message sink.
// It should provide no way to query anything about the outside world.
struct IPresenter
{
  virtual ~IPresenter() = default;

  virtual void preload(Resource res) = 0;
  virtual void textBox(String msg) = 0;
  virtual void playMusic(MUSIC id) = 0;
  virtual void stopMusic() = 0;
  virtual void playSound(SOUND id) = 0;
  virtual void setCameraPos(Vec2f pos, bool teleport = false) = 0;
  virtual void setAmbientLight(float amount) = 0;

  // adds a displayable object to the current frame
  virtual void sendActor(SpriteActor const& actor) = 0;
  virtual void sendActor(DebugRectActor const& actor) = 0;

  virtual void flushFrame() = 0;
};

