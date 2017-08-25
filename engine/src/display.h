#pragma once

#include "base/geom.h"

void Display_init(int width, int height);
void Display_setCaption(const char* caption);
void Display_loadModel(int id, const char* imagePath);
void Display_beginDraw();
void Display_endDraw();
void Display_drawActor(Rect2f where, int modelId, bool blinking, int actionIdx, float frame);
void Display_drawText(Vector2f pos, char const* text);

