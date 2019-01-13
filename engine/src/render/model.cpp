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

extern int loadTexture(const char* path, Rect2f rect);

static
void addTexture(Action& action, const char* path, Rect2f rect)
{
  action.textures.push_back(loadTexture(path, rect));
}

static
Action loadSheetAction(json::Value* val, string sheetPath, int ROWS, int COLS)
{
  Action r;

  auto action = json::cast<json::Object>(val);
  action->getMember<json::String>("name");
  auto frames = action->getMember<json::Array>("frames");

  for(auto& frame : frames->elements)
  {
    auto const idx = (json::cast<json::Number>(frame.get()))->value;

    auto const col = idx % COLS;
    auto const row = idx / COLS;
    Rect2f rect;
    rect.pos.x = col / float(COLS);
    rect.pos.y = row / float(ROWS);
    rect.size.width = 1.0 / float(COLS);
    rect.size.height = 1.0 / float(ROWS);
    addTexture(r, sheetPath.c_str(), rect);
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
  auto sheet = obj->getMember<json::String>("sheet")->value;
  auto const cols = obj->getMember<json::Number>("cols")->value;
  auto const rows = obj->getMember<json::Number>("rows")->value;

  if(type == "sheet")
  {
    auto actions = obj->getMember<json::Array>("actions");

    for(auto& action : actions->elements)
      r.actions.push_back(loadSheetAction(action.get(), dir + "/" + sheet, rows, cols));
  }
  else if(type == "tiled")
  {
    for(int row = 0; row < rows; ++row)
    {
      for(int col = 0; col < rows; ++col)
      {
        Action action;
        Rect2f rect;
        rect.pos.x = col / float(cols);
        rect.pos.y = row / float(rows);
        rect.size.width = 1.0 / float(cols);
        rect.size.height = 1.0 / float(rows);
        addTexture(action, (dir + "/" + sheet).c_str(), rect);
        r.actions.push_back(action);
      }
    }
  }
  else
    throw runtime_error("Unknown model type: '" + type + "'");

  return r;
}

static
Model loadTiledModel(const char* path, int count, int COLS, int ROWS)
{
  auto m = Model();

  for(int i = 0; i < count; ++i)
  {
    auto col = i % COLS;
    auto row = i / COLS;

    auto const width = 1.0 / float(COLS);
    auto const height = 1.0 / float(ROWS);

    Action action;
    addTexture(action, path, Rect2f(col * width, row * height, width, height));
    m.actions.push_back(action);
  }

  return m;
}

Model loadModel(const char* path)
{
  try
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
  catch(std::exception const& e)
  {
    throw runtime_error("When loading '" + string(path) + "': " + e.what());
  }
}

