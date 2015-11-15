#include "model.h"
#include "geom.h"
#include "json.h"
#include "util.h"

static
void loadFrame(Model& r, json::Value* val, string dir)
{
  auto frame = json::cast<json::Object>(val);
  auto idx = frame->getMember<json::String>("idx");
  auto path = dir + "/" + idx->value + ".png";
  r.addTexture(path);
}

static
void loadAction(Model& r, json::Value* val, string dir)
{
  auto action = json::cast<json::Object>(val);
  auto name = action->getMember<json::String>("name");
  auto frames = action->getMember<json::Array>("frames");

  for(auto& frame : frames->elements)
    loadFrame(r, frame.get(), dir);
}

Model loadModel(string jsonPath)
{
  Model r;
  auto obj = json::load(jsonPath);
  auto dir = dirName(jsonPath);

  auto actions = obj->getMember<json::Array>("actions");

  for(auto& action : actions->elements)
    loadAction(r, action.get(), dir);

  return r;
}

int loadTexture(string path, Rect2i rect = Rect2i(0, 0, 0, 0));

void Model::addTexture(string path, Rect2i rect)
{
  textures.push_back(loadTexture(path, rect));
}

