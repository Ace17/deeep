// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "model.h"
#include "base/geom.h"
#include "misc/util.h" // dirName
#include "misc/json.h"
#include "misc/file.h"

extern int loadTexture(const char* path, Rect2i rect);

static
void addTexture(Action& action, const char* path, Rect2i rect)
{
  action.textures.push_back(loadTexture(path, rect));
}

static
Action loadSheetAction(json::Value* val, string sheetPath, Size2i cell)
{
  Action r;

  auto action = json::cast<json::Object>(val);
  action->getMember<json::String>("name");
  auto frames = action->getMember<json::Array>("frames");

  for(auto& frame : frames->elements)
  {
    auto const idx = (json::cast<json::Number>(frame.get()))->value;

    auto const col = idx % 16;
    auto const row = idx / 16;
    addTexture(r, sheetPath.c_str(), Rect2i(col * cell.width, row * cell.height, cell.width, cell.height));
  }

  return r;
}

static
Model loadAnimatedModel(const char* jsonPath)
{
  auto data = read(jsonPath);
  Model r;
  auto obj = json::parse(data.c_str(), data.size());
  auto dir = dirName(jsonPath);

  auto type = obj->getMember<json::String>("type")->value;
  auto actions = obj->getMember<json::Array>("actions");

  if(type != "sheet")
    throw runtime_error("Unknown model type: '" + type + "'");

  auto sheet = obj->getMember<json::String>("sheet")->value;
  auto width = obj->getMember<json::Number>("width")->value;
  auto height = obj->getMember<json::Number>("height")->value;

  auto cell = Size2i(width, height);

  for(auto& action : actions->elements)
    r.actions.push_back(loadSheetAction(action.get(), dir + "/" + sheet, cell));

  return r;
}

Model loadTiledModel(const char* path, int count, int COLS, int SIZE)
{
  auto m = Model();

  for(int i = 0; i < count; ++i)
  {
    auto col = i % COLS;
    auto row = i / COLS;

    Action action;
    addTexture(action, path, Rect2i(col * SIZE, row * SIZE, SIZE, SIZE));
    m.actions.push_back(action);
  }

  return m;
}

Model loadModel(const char* path)
{
  if(endsWith(path, ".model"))
  {
    if(!exists(path))
    {
      printf("[display] model '%s' doesn't exist, fallback on default model\n", path);
      path = "res/sprites/rect.model";
    }

    return loadAnimatedModel(path);
  }
  else if(endsWith(path, ".tiles"))
  {
    auto pngPath = setExtension(path, "png");

    if(!exists(pngPath))
    {
      printf("[display] tileset '%s' was not found, fallback on default tileset\n", pngPath.c_str());
      pngPath = "res/tiles/default.png";
    }

    return loadTiledModel(pngPath.c_str(), 64 * 2, 8, 16);
  }
  else
  {
    throw runtime_error("unknown format for '" + string(path) + "'");
  }
}

