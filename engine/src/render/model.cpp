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

extern int loadTexture(string path, Rect2i rect = Rect2i(0, 0, 0, 0));

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
    addTexture(r, sheetPath, Rect2i(col * cell.width, row * cell.height, cell.width, cell.height));
  }

  return r;
}

Model loadModel(string jsonPath)
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

void addTexture(Action& action, string path, Rect2i rect)
{
  action.textures.push_back(loadTexture(path, rect));
}

