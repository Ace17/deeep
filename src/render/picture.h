#pragma once

#include <cstdint>
#include <vector>

#include "base/geom.h"

struct PictureView
{
  Size2i dim;
  int stride;
  uint8_t* pixels;
};

struct Picture
{
  Size2i dim;
  int stride;
  std::vector<uint8_t> pixels;

  operator PictureView () { return { dim, stride, pixels.data() }; }
};

Picture loadPicture(String path, Rect2f frect);

