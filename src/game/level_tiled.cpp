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
#include <fstream>
#include "base/geom.h"
#include "base/util.h"
#include "engine/json.h"
#include "engine/base64.h"
#include "engine/decompress.h"
#include "game/room.h"

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

static
Size2i getSize(json::Object* obj)
{
  Size2i r;

  r.width = obj->getMember<json::Number>("width")->value;
  r.height = obj->getMember<json::Number>("height")->value;

  return r;
}

Rect2i getRect(json::Object* obj)
{
  Rect2i r;

  r.x = obj->getMember<json::Number>("x")->value;
  r.y = obj->getMember<json::Number>("y")->value;

  Size2i& s = r;
  s = getSize(obj);

  return r;
}

static
map<string, json::Object*> getAllLayers(json::Object* js)
{
  auto layers = js->getMember<json::Array>("layers");

  map<string, json::Object*> nameToLayer;

  for(auto& layer : layers->elements)
  {
    auto lay = json::cast<json::Object>(layer.get());
    auto name = lay->getMember<json::String>("name")->value;
    nameToLayer[name] = lay;
  }

  return nameToLayer;
}

static
Matrix<int> parseTileLayer(json::Object* json)
{
  Matrix<int> tiles;

  auto const data = json->getMember<json::String>("data")->value;
  auto const buff = decompressTiles(data);
  auto const size = getSize(json);

  if(size.width * size.height != (int)buff.size())
    throw runtime_error("invalid TMX file: width x height doesn't match data length");

  tiles.resize(size);

  for(auto pos : rasterScan(size.width, size.height))
  {
    auto const x = pos.first;
    auto const y = pos.second;
    tiles.set(x, y, 0);

    int srcOffset = (x + (size.height - 1 - y) * size.width);
    int tile = buff[srcOffset];

    assert(tile >= 0);

    if(tile)
    {
      auto const abstractTile = 1 + ((tile - 1) / 16);
      tiles.set(x, y, abstractTile);
    }
  }

  return tiles;
}

Room parseRoom(json::Object* jsonRoom)
{
  auto layers = getAllLayers(jsonRoom);
  json::Object* tileLayer {};
  json::Object* objectLayer {};

  Room room;

  const auto rect = getRect(tileLayer);

  room.pos = rect;

  room.tiles = parseTileLayer(tileLayer);

  if(objectLayer)
  {
    auto objects = objectLayer->getMember<json::Array>("objects");

    for(auto& jsonObj : objects->elements)
    {
      auto obj = json::cast<json::Object>(jsonObj.get());
      auto const objRect = getRect(obj);

      if(obj->getMember<json::String>("name")->value == "start")
      {
        room.start.x = objRect.x / 16;
        room.start.y = rect.height - 1 - (objRect.y + objRect.height) / 16;
      }
    }
  }

  room.start -= room.pos;

  return room;
}

void generateBasicRoom(Room& room)
{
  auto const rect = room.tiles.size;

  for(int x = 0; x < rect.width; ++x)
  {
    // ground
    room.tiles.set(x, 0, 1);

    // ceiling
    for(int i = 0; i < 6; ++i)
      room.tiles.set(x, rect.height - 1 - i, 1);
  }

  for(int y = 0; y < min(3, rect.height - 1); ++y)
  {
    room.tiles.set(0, y, 1);
    room.tiles.set(rect.width - 1, y, 1);
  }

  for(int y = 0; y < rect.height - 1; ++y)
    for(int x = 0; x < rect.width - 1; ++x)
    {
      auto const c = x / 16;
      auto const col = x % 16;
      auto const row = y % 16;

      if(col >= 7 && col < 9 && (row + c) % 4 == 0)
        room.tiles.set(x, y, 1);

      if(col >= 11 && col < 13 && (row + c) % 4 == 2)
        room.tiles.set(x, y, 1);

      if(row == 1 && (col < 4 || col >= 12))
        room.tiles.set(x, y, 1);
    }
}

vector<Room> loadQuest(string path) // tiled TMX format
{
  auto js = json::load(path);

  auto layers = getAllLayers(js.get());

  auto layer = layers["rooms"];

  if(!layer)
    throw runtime_error("room layer was not found");

  vector<Room> r;

  for(auto& roomValue : layer->getMember<json::Array>("objects")->elements)
  {
    auto jsonRoom = json::cast<json::Object>(roomValue.get());
    auto rect = getRect(jsonRoom);

    {
      // tiled stores dimensions as pixel units
      // convert them back to logical units (i.e tile units)
      auto const PELS_PER_TILE = 4;
      rect.x /= PELS_PER_TILE;
      rect.y /= PELS_PER_TILE;
      rect.width /= PELS_PER_TILE;
      rect.height /= PELS_PER_TILE;

      // tiled uses a downwards pointing Y axis.
      // reverse it so it points upwards.
      rect.y = 64 - rect.y - rect.height;
    }

    auto const CELL_SIZE = 16;
    auto const tilemapSize = Size2i(rect.width, rect.height) * CELL_SIZE;

    Room room;
    room.name = jsonRoom->getMember<json::String>("name")->value;
    room.pos = rect;
    room.size = rect;
    room.tiles.resize(tilemapSize);
    room.start = Vector2i(tilemapSize.width / 2, tilemapSize.height / 4);

    {
      auto const path = "res/rooms/" + room.name + ".json";

      if(ifstream(path).is_open())
      {
        auto jsRoom = json::load(path);
        auto layers = getAllLayers(jsRoom.get());
        assert(layers["tiles"]);
        room.tiles = parseTileLayer(layers["tiles"]);
      }
      else
        generateBasicRoom(room);
    }

    r.push_back(move(room));
  }

  return r;
}

