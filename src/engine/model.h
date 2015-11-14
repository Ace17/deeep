#pragma once

#include <stdint.h>
#include <vector>
#include <string>
using namespace std;

#include "geom.h"

struct Model
{
  uint32_t buffer = 0;
  uint32_t indices = 0;
  int numIndices = 0;
  int size = 0;
  vector<int> textures;
  float scale = 1;

  void addTexture(string path, Rect2i rect = Rect2i(0, 0, 0, 0));
};

Model loadModel(string jsonPath);

