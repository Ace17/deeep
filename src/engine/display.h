// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// interface to a high-level renderer

#pragma once

#include <stdint.h>

#include "base/box.h"
#include "base/geom.h"
#include "base/span.h"
#include "base/string.h"

struct Display
{
  virtual ~Display() = default;

  virtual void loadModel(int id, String imagePath) = 0;
  virtual void setCamera(Vector2f pos) = 0;
  virtual void setAmbientLight(float ambientLight) = 0;

  // draw functions
  virtual void beginDraw() = 0;
  virtual void endDraw() = 0;
  virtual void drawActor(Rect2f where, float angle, bool useWorldRefFrame, int modelId, bool blinking, int actionIdx, float frame, int zOrder) = 0;
  virtual void drawText(Vector2f pos, char const* text) = 0;
};

