/*
 * Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include "base/geom.h"

struct Display
{
  virtual ~Display() = default;

  virtual void init(Size2i resolution) = 0;
  virtual void setFullscreen(bool fs) = 0;
  virtual void setCaption(const char* caption) = 0;
  virtual void loadModel(int id, const char* imagePath) = 0;
  virtual void beginDraw() = 0;
  virtual void endDraw() = 0;
  virtual void drawActor(Rect2f where, int modelId, bool blinking, int actionIdx, float frame) = 0;
  virtual void drawText(Vector2f pos, char const* text) = 0;
  virtual void setAmbientLight(float ambientLight) = 0;
};

