#include "model.h"
#include "geom.h"
#include "json.h"

Model loadModel(string jsonPath)
{
  Model m;
  auto obj = json::load(jsonPath);
  return m;
}

int loadTexture(string path, Rect2i rect = Rect2i(0, 0, 0, 0));

void Model::addTexture(string path, Rect2i rect)
{
  textures.push_back(loadTexture(path, rect));
}

