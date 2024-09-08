// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "base/box.h"
#include "base/error.h"
#include "base/geom.h"
#include "base/logger.h"
#include "base/string.h"
#include "misc/file.h"
#include "misc/json.h"
#include "misc/util.h" // dirName
#include "model.h"

static
void addTexture(Action& action, String path, Rect2f rect, const LoadTextureFunc& loadTexture)
{
  action.textures.push_back(loadTexture(path, rect));
}

static
Action loadSheetAction(json::Value const& action, std::string sheetPath, int ROWS, int COLS, const LoadTextureFunc& loadTexture)
{
  Action r;

  action["name"];

  for(auto& frame : action["frames"].elements)
  {
    auto const idx = (int)frame;

    auto const col = idx % COLS;
    auto const row = idx / COLS;
    Rect2f rect;
    rect.pos.x = col / float(COLS);
    rect.pos.y = row / float(ROWS);
    rect.size.x = 1.0 / float(COLS);
    rect.size.y = 1.0 / float(ROWS);
    addTexture(r, sheetPath, rect, loadTexture);
  }

  return r;
}

static
Model loadAnimatedModel(String jsonPath, const LoadTextureFunc& loadTexture)
{
  auto data = File::read(jsonPath);
  Model r;
  auto obj = json::parse(data.c_str(), data.size());

  auto const cols = (int)obj["cols"];
  auto const rows = (int)obj["rows"];

  std::string sheet = setExtension(jsonPath, "png");

  if(obj.has("actions"))
  {
    for(auto& action : obj["actions"].elements)
      r.actions.push_back(loadSheetAction(action, sheet, rows, cols, loadTexture));
  }
  else
  {
    for(int row = 0; row < rows; ++row)
    {
      for(int col = 0; col < cols; ++col)
      {
        Action action;
        Rect2f rect;
        rect.pos.x = col / float(cols);
        rect.pos.y = row / float(rows);
        rect.size.x = 1.0 / float(cols);
        rect.size.y = 1.0 / float(rows);

        addTexture(action, sheet, rect, loadTexture);
        r.actions.push_back(action);
      }
    }
  }

  return r;
}

static
Model loadTiledModel(String path, int count, int COLS, int ROWS, const LoadTextureFunc& loadTexture)
{
  auto m = Model();

  for(int i = 0; i < count; ++i)
  {
    auto col = i % COLS;
    auto row = i / COLS;

    Rect2f rect;
    rect.size.x = 1.0 / float(COLS);
    rect.size.y = 1.0 / float(ROWS);
    rect.pos.x = col * rect.size.x;
    rect.pos.y = row * rect.size.y;

    Action action;
    addTexture(action, path, rect, loadTexture);
    m.actions.push_back(action);
  }

  return m;
}

Model loadModel(String path, const LoadTextureFunc& loadTexture)
{
  try
  {
    if(endsWith(path, ".model"))
    {
      if(!File::exists(path))
      {
        logMsg("[display] model '%.*s' doesn't exist, fallback on default model", path.len, path.data);
        path = "res/sprites/rect.model";
      }

      return loadAnimatedModel(path, loadTexture);
    }
    else if(endsWith(path, ".tiles"))
    {
      auto pngPath = setExtension(path, "png");

      if(!File::exists(pngPath))
      {
        logMsg("[display] tileset '%s' was not found, fallback on default tileset", pngPath.c_str());
        pngPath = "res/tiles/default.png";
      }

      return loadTiledModel(pngPath, 64 * 2, 8, 16, loadTexture);
    }
    else
    {
      throw Error("unknown format for '" + std::string(path.data, path.len) + "'");
    }
  }
  catch(const Error& e)
  {
    throw Error("When loading '" + std::string(path.data, path.len) + "': " + std::string(e.message().data, e.message().len));
  }
}

