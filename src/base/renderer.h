// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// interface to a high-level renderer

#pragma once

#include <stdint.h>

#include "box.h"
#include "geom.h"
#include "span.h"
#include "string.h"

struct RenderSprite
{
  Vec2f pos;
  bool useWorldRefFrame;
  int zOrder;
  float angle;
  Vec2f halfSize;
  bool blinking;
  int modelId;
  int actionIdx;
  float frame;
};

struct RenderText
{
  Vec2f pos;
  String text;
};

struct IRenderer
{
  virtual ~IRenderer() = default;

  virtual void loadModel(int id, String imagePath) = 0;
  virtual void setCamera(Vec2f pos) = 0;
  virtual void setAmbientLight(float ambientLight) = 0;

  // draw functions
  virtual void beginDraw() = 0;
  virtual void endDraw() = 0;
  virtual void drawSprite(const RenderSprite& sprite) = 0;
  virtual void drawText(const RenderText& text) = 0;
};

