// Copyright (C) 2023 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// interface to a high-level renderer

#pragma once

#include <stdint.h>

#include "geom.h"
#include "span.h"
#include "string.h"

struct RenderColor
{
  float r, g, b, a;
};

struct RenderObject
{
  bool useWorldRefFrame;
  int zOrder;
};

struct RenderSprite : RenderObject
{
  Vec2f pos;
  float angle;
  Vec2f halfSize;
  bool blinking;
  int modelId;
  int actionIdx;
  float frame;
};

struct RenderText : RenderObject
{
  Vec2f pos;
  String text;
};

struct RenderCircle : RenderObject
{
  Vec2f pos;
  float radius;
  RenderColor color;
};

struct RenderLine : RenderObject
{
  Vec2f a, b;
  RenderColor color;
  float thicknessMin = 1;
  float thicknessMax = 1;
};

struct IRenderer
{
  virtual ~IRenderer() = default;

  virtual void loadModel(int id, String imagePath) = 0;
  virtual void setCamera(Vec2f pos, bool teleport) = 0;
  virtual void setAmbientLight(float ambientLight) = 0;

  // draw functions
  virtual void beginDraw() = 0;
  virtual void endDraw() = 0;
  virtual void drawSprite(const RenderSprite& sprite) = 0;
  virtual void drawText(const RenderText& text) = 0;
  virtual void drawCircle(const RenderCircle& circle) = 0;
  virtual void drawLine(const RenderLine& line) = 0;
};

