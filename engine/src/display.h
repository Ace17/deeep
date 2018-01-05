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
};

