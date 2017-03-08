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
Size2i getSize(json::Object* obj)
{
  Size2i r;

  r.width = obj->getMember<json::Number>("width")->value;
  r.height = obj->getMember<json::Number>("height")->value;

  return r;
}

static
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
Rect2i convertRect(Rect2i rect, int pelsPerTile, int areaHeight)
{
  // tiled stores dimensions as pixel units
  // convert them back to logical units (i.e tile units)
  rect.x /= pelsPerTile;
  rect.y /= pelsPerTile;
  rect.width /= pelsPerTile;
  rect.height /= pelsPerTile;

  // tiled uses a downwards pointing Y axis.
  // reverse it so it points upwards.
  rect.y = areaHeight - rect.y - rect.height;

  return rect;
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
vector<Room::Thing> parseThingLayer(json::Object* objectLayer, int height)
{
  vector<Room::Thing> r;
  auto objects = objectLayer->getMember<json::Array>("objects");

  for(auto& jsonObj : objects->elements)
  {
    auto obj = json::cast<json::Object>(jsonObj.get());
    auto const objRect = convertRect(getRect(obj), 16, height);

    auto const name = obj->getMember<json::String>("name")->value;
    auto const pos = Vector2f(objRect.x, objRect.y);

    r.push_back(Room::Thing { pos, name });
  }

  return r;
}

static
void loadConcreteRoom(Room& room, json::Object* jsRoom)
{
  auto layers = getAllLayers(jsRoom);
  assert(layers["tiles"]);
  room.tiles = parseTileLayer(layers["tiles"]);

  if(exists(layers, "things"))
    room.things = parseThingLayer(layers["things"], room.size.height * 16);

  // add spikes
  for(auto pos : rasterScan(room.tiles.size.width, room.tiles.size.height))
  {
    auto const x = pos.first;
    auto const y = pos.second;

    if(room.tiles.get(x, y) >= 8)
    {
      auto const pos = Vector2f(x, y);
      room.things.push_back(Room::Thing { pos, "spikes" });
      room.tiles.set(x, y, 0);
    }
  }
}

static
Room loadAbstractRoom(json::Object* jsonRoom)
{
  auto rect = getRect(jsonRoom);

  auto const PELS_PER_TILE = 4;
  rect = convertRect(rect, PELS_PER_TILE, 64);

  auto const sizeInTiles = Size2i(rect.width, rect.height) * CELL_SIZE;

  Room room;
  room.name = jsonRoom->getMember<json::String>("name")->value;
  room.pos = rect;
  room.size = rect;
  room.start = Vector2i(sizeInTiles.width / 2, sizeInTiles.height / 4);
  room.theme = atoi(jsonRoom->getMember<json::String>("type")->value.c_str());

  auto const path = "res/rooms/" + room.name + ".json";

  if(ifstream(path).is_open())
  {
    auto jsRoom = json::load(path);
    loadConcreteRoom(room, jsRoom.get());
  }
  else
  {
    generateConcreteRoom(room);
  }

  return room;
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

    auto room = loadAbstractRoom(jsonRoom);
    r.push_back(move(room));
  }

  return r;
}

