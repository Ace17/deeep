/*
 * Copyright (C) 2015 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <cassert>
#include <map>
#include <string>
#include "base/geom.h"
#include "base/util.h"
#include "engine/json.h"
#include "engine/base64.h"
#include "engine/decompress.h"
#include "game/game.h"
#include "game/entities/switch.h"
#include "game/entities/wheel.h"
#include "game/level_graph.h"

static
vector<int> convertFromLittleEndian(vector<uint8_t> const& input)
{
  assert(input.size() % 4 == 0);
  vector<int> r(input.size() / 4);

  for(int i = 0; i < (int)r.size(); ++i)
  {
    r[i] = 0;
    r[i] += input[i * 4 + 0] << 0;
    r[i] += input[i * 4 + 1] << 8;
    r[i] += input[i * 4 + 2] << 16;
    r[i] += input[i * 4 + 3] << 24;
  }

  return r;
}

vector<int> decompressTiles(string data)
{
  while(data.size() % 4)
    data += "=";

  auto const compDataBuffer = decodeBase64(data);
  Span<const uint8_t> compData;
  compData.data = compDataBuffer.data();
  compData.len = compDataBuffer.size();

  auto const uncompData = decompress(compData);
  return convertFromLittleEndian(uncompData);
}

Level parseLevel(json::Object* tileLayer, json::Object* objectLayer)
{
  Level level;

  const auto width = tileLayer->getMember<json::Number>("width")->value;
  const auto height = tileLayer->getMember<json::Number>("height")->value;

  {
    auto data = tileLayer->getMember<json::String>("data")->value;

    level.pos.x = tileLayer->getMember<json::Number>("x")->value;
    level.pos.y = tileLayer->getMember<json::Number>("y")->value;

    auto buff = decompressTiles(data);

    if(width * height != (int)buff.size())
      throw runtime_error("invalid TMX file: width x height doesn't match data length");

    level.tiles.resize(Size2i(width, height));

    for(auto pos : rasterScan(width, height))
    {
      auto const x = pos.first;
      auto const y = pos.second;
      level.tiles.set(x, y, 0);

      int srcOffset = (x + (height - 1 - y) * width);
      int tile = buff[srcOffset];

      assert(tile >= 0);

      if(tile)
      {
        auto const abstractTile = 1 + ((tile - 1) / 16);
        level.tiles.set(x, y, abstractTile);
      }
    }
  }

  if(objectLayer)
  {
    auto objects = objectLayer->getMember<json::Array>("objects");

    for(auto& jsonObj : objects->elements)
    {
      auto obj = json::cast<json::Object>(jsonObj.get());
      auto const objx = obj->getMember<json::Number>("x")->value;
      auto const objy = obj->getMember<json::Number>("y")->value;
      auto const objHeight = obj->getMember<json::Number>("height")->value;

      if(obj->getMember<json::String>("name")->value == "start")
      {
        level.start.x = objx / 16;
        level.start.y = height - 1 - (objy + objHeight) / 16;
      }
    }
  }

  level.start -= level.pos;

  return level;
}

vector<Level> loadQuest(string path) // tiled TMX format
{
  vector<Level> r;

  struct JsonLevel
  {
    json::Object* tileLayer;
    json::Object* objectLayer;
  };

  json::Object* portalLayer = nullptr;

  map<string, JsonLevel> jsonLevels;

  auto js = json::load(path);
  auto layers = js->getMember<json::Array>("layers");

  for(auto& layer : layers->elements)
  {
    auto lay = json::cast<json::Object>(layer.get());
    auto name = lay->getMember<json::String>("name")->value;
    auto type = lay->getMember<json::String>("type")->value;

    if(name == "portals")
      portalLayer = lay;
    else if(type == "tilelayer")
      jsonLevels[name].tileLayer = lay;
    else
      jsonLevels[name].objectLayer = lay;
  }

  for(auto jsonLevel : jsonLevels)
  {
    if(!jsonLevel.second.tileLayer)
      continue;

    r.push_back(parseLevel(jsonLevel.second.tileLayer, jsonLevel.second.objectLayer));
  }

  if(portalLayer)
  {
    auto objects = portalLayer->getMember<json::Array>("objects");

    for(auto& portal : objects->elements)
    {
      (void)portal;
    }
  }

  return r;
}

