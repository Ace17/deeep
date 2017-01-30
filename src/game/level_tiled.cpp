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

Level parseLevel(json::Object* tileLayer, json::Object* objects)
{
  Level level;

  auto data = tileLayer->getMember<json::String>("data")->value;

  auto buff = decompressTiles(data);

  const auto width = tileLayer->getMember<json::Number>("width")->value;
  const auto height = tileLayer->getMember<json::Number>("height")->value;

  if(width * height != (int)buff.size())
    throw runtime_error("invalid TMX file: width x height doesn't match data length");

  level.tiles.resize(Size2i(width / 2, height / 2));

  for(int y = 0; y < height / 2; ++y)
  {
    for(int x = 0; x < width / 2; ++x)
    {
      level.tiles.set(x, y, 0);

      int srcOffset = (x * 2 + (height - 1 - y * 2) * width);
      int tile = buff[srcOffset];

      assert(tile >= 0);

      if(tile)
      {
        auto const abstractTile = 1 + ((tile - 1) / 16);
        level.tiles.set(x, y, abstractTile);
      }
    }
  }

  level.start = Vector2i(58, 20);

  if(objects)
  {
  }

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

  map<string, JsonLevel> jsonLevels;

  auto js = json::load(path);
  auto layers = js->getMember<json::Array>("layers");

  for(auto& layer : layers->elements)
  {
    auto lay = json::cast<json::Object>(layer.get());
    auto name = lay->getMember<json::String>("name")->value;
    auto type = lay->getMember<json::String>("type")->value;

    if(type == "tilelayer")
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

  return r;
}

