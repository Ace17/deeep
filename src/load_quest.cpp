// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Loader for the quest file and room files.
// (using the TMX JSON+gzip format)
#include <cassert>
#include <map>
#include <string>
#include "base/geom.h"
#include "base/util.h"

#include "engine/src/misc/file.h"
#include "engine/src/misc/json.h"
#include "engine/src/misc/base64.h"
#include "engine/src/misc/decompress.h"

#include "quest.h"

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

static
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
Size2i getSize(json::Value const& obj)
{
  Size2i r;

  r.width = (int)obj["width"];
  r.height = (int)obj["height"];

  return r;
}

static
Rect2i getRect(json::Value const& obj)
{
  Rect2i r;

  r.pos.x = (int)obj["x"];
  r.pos.y = (int)obj["y"];
  r.size = getSize(obj);

  return r;
}

static
Rect2i convertRect(Rect2i rect, int pelsPerTile, int areaHeight)
{
  // tiled stores dimensions as pixel units
  // convert them back to logical units (i.e tile units)
  rect.pos.x /= pelsPerTile;
  rect.pos.y /= pelsPerTile;
  rect.size.width /= pelsPerTile;
  rect.size.height /= pelsPerTile;

  // tiled uses a downwards pointing Y axis.
  // reverse it so it points upwards.
  rect.pos.y = areaHeight - rect.pos.y - rect.size.height;

  return rect;
}

static
map<string, json::Value> getAllLayers(json::Value const& js)
{
  map<string, json::Value> nameToLayer;

  for(auto& layer : js["layers"].elements)
  {
    auto name = (string)layer["name"];
    nameToLayer[name] = layer;
  }

  return nameToLayer;
}

static
Matrix2<int> parseTileLayer(json::Value& json)
{
  Matrix2<int> tiles;

  auto const data = (string)json["data"];
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

    tiles.set(x, y, tile);
  }

  return tiles;
}

static auto const CELL_SIZE = 16;

static
void generateConcreteRoom(Room& room)
{
  auto const rect = room.size * CELL_SIZE;
  room.tiles.resize(Size2i(rect.width, rect.height));

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

  for(int y = 0; y < rect.height; ++y)
    for(int x = 0; x < rect.width; ++x)
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

      if(x == 0 || x == rect.width - 1)
        if(row < 3 || row >= 6)
          room.tiles.set(x, y, 1);
    }
}

static
vector<Room::Spawner> parseThingLayer(json::Value const& objectLayer, int height)
{
  vector<Room::Spawner> r;

  for(auto& obj : objectLayer["objects"].elements)
  {
    auto const objRect = convertRect(getRect(obj), 16, height);

    auto const name = (string)obj["name"];
    auto const pos = Vector(objRect.pos.x, objRect.pos.y);

    r.push_back(Room::Spawner { pos, name });
  }

  return r;
}

static
void loadConcreteRoom(Room& room, json::Value const& jsRoom)
{
  auto layers = getAllLayers(jsRoom);
  room.tiles = parseTileLayer(layers["tiles"]);

  if(exists(layers, "things"))
    room.spawners = parseThingLayer(layers["things"], room.size.height * 16);

  // add spikes
  for(auto pos : rasterScan(room.tiles.size.width, room.tiles.size.height))
  {
    auto const x = pos.first;
    auto const y = pos.second;

    if(room.tiles.get(x, y) >= 8)
    {
      auto const pos = Vector(x, y);
      room.spawners.push_back({ pos, "spikes" });
      room.tiles.set(x, y, 0);
    }
  }
}

static
Room loadAbstractRoom(json::Value const& jsonRoom)
{
  auto box = getRect(jsonRoom);

  auto const PELS_PER_TILE = 4;
  box = convertRect(box, PELS_PER_TILE, 64);

  auto const sizeInTiles = box.size * CELL_SIZE;

  Room room;
  room.name = (string)jsonRoom["name"];
  room.pos = box.pos;
  room.size = box.size;
  room.start = Vector2i(sizeInTiles.width / 2, sizeInTiles.height / 4);
  room.theme = atoi(string(jsonRoom["type"]).c_str());

  auto const path = "res/rooms/" + room.name + ".json";

  if(exists(path))
  {
    auto data = read(path);
    auto jsRoom = json::parse(data.c_str(), data.size());
    loadConcreteRoom(room, jsRoom);
  }
  else
  {
    generateConcreteRoom(room);
  }

  auto const actualSize = room.tiles.size / CELL_SIZE;

  if(actualSize != room.size)
  {
    char buffer[256];
    sprintf(buffer, "room instance at (%d;%d) with theme %s/%d has wrong dimensions: map expected %dx%d, but the concrete tileset is %dx%d\n",
            room.pos.x, room.pos.y,
            path.c_str(), room.theme,
            room.size.width, room.size.height,
            actualSize.width, actualSize.height);
    throw runtime_error(buffer);
  }

  return room;
}

Quest loadQuest(string path) // tiled TMX format
{
  auto data = read(path);
  auto js = json::parse(data.c_str(), data.size());

  auto layers = getAllLayers(js);

  auto layer = layers["rooms"];

  Quest r;

  for(auto& roomValue : layer["objects"].elements)
  {
    auto room = loadAbstractRoom(roomValue);
    r.rooms.push_back(move(room));
  }

  return r;
}

