// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Loader for the quest file and room files.
// (using the TMX JSON+gzip format)
#include "base/box.h"
#include "base/error.h"
#include "base/geom.h"
#include "base/util.h"
#include <cassert>
#include <map>
#include <string>

#include "misc/base64.h"
#include "misc/decompress.h"
#include "misc/file.h"
#include "misc/json.h"
#include "misc/util.h" // baseName, removeExtension

#include "quest.h"

static
vector<int> convertFromLittleEndian(Span<const uint8_t> input)
{
  assert(input.len % 4 == 0);
  vector<int> r(input.len / 4);

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

  auto const uncompData = zlibDecompress(compData);
  return convertFromLittleEndian(uncompData);
}

static
Vec2i getSize(json::Value const& obj)
{
  return Vec2i(obj["width"], obj["height"]);
}

static
Rect2i getRect(json::Value const& obj)
{
  Rect2i r;

  r.pos = Vec2i(obj["x"], obj["y"]);
  r.size = getSize(obj);

  return r;
}

static const int TiledPixelsPerTile = 16;

static
Rect2i transformTiledRect(Rect2i rect, int areaHeight)
{
  // tiled stores dimensions as pixel units
  // convert them back to logical units (i.e tile units)
  rect.pos.x /= TiledPixelsPerTile;
  rect.pos.y /= TiledPixelsPerTile;
  rect.size.x /= TiledPixelsPerTile;
  rect.size.y /= TiledPixelsPerTile;

  // tiled uses a downwards pointing Y axis.
  // reverse it so it points upwards.
  rect.pos.y = areaHeight - rect.pos.y - rect.size.y;

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

  if(size.x * size.y != (int)buff.size())
    throw Error("invalid TMX file: width x height doesn't match data length");

  tiles.resize(size);

  for(auto pos : rasterScan(size.x, size.y))
  {
    auto const x = pos.first;
    auto const y = pos.second;
    tiles.set(x, y, 0);

    int srcOffset = (x + (size.y - 1 - y) * size.x);
    int tile = buff[srcOffset];

    assert(tile >= 0);

    tiles.set(x, y, tile);
  }

  return tiles;
}

extern const Vec2i CELL_SIZE { 16, 16 };

static
void generateConcreteRoom(Room& room)
{
  const Vec2i rect { room.size.x * CELL_SIZE.x, room.size.y * CELL_SIZE.y };
  room.tiles.resize(rect);

  for(int x = 0; x < rect.x; ++x)
  {
    // ground
    room.tiles.set(x, 0, 1);

    // ceiling
    for(int i = 0; i < 6; ++i)
      room.tiles.set(x, rect.y - 1 - i, 1);
  }

  for(int y = 0; y < min(3, rect.y - 1); ++y)
  {
    room.tiles.set(0, y, 1);
    room.tiles.set(rect.x - 1, y, 1);
  }

  for(int y = 0; y < rect.y; ++y)
    for(int x = 0; x < rect.x; ++x)
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

      if(x == 0 || x == rect.x - 1)
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
    auto const objRect = transformTiledRect(getRect(obj), height);

    Room::Spawner spawner;

    spawner.pos = Vector(objRect.pos.x, objRect.pos.y);
    spawner.name = (string)obj["name"];
    spawner.id = obj["id"];

    if(obj.has("properties"))
    {
      for(auto& prop : obj["properties"].elements)
      {
        auto varName = (string)prop["name"];
        auto varValue = (string)prop["value"];
        spawner.config[varName] = varValue;
      }
    }

    r.push_back(spawner);
  }

  return r;
}

static
void loadConcreteRoom(Room& room, json::Value const& jsRoom)
{
  auto layers = getAllLayers(jsRoom);
  room.tiles = parseTileLayer(layers["tiles"]);

  if(exists(layers, "things"))
    room.spawners = parseThingLayer(layers["things"], room.size.y * CELL_SIZE.y);

  // add spikes and ladders
  for(auto pos : rasterScan(room.tiles.size.x, room.tiles.size.y))
  {
    auto const x = pos.first;
    auto const y = pos.second;
    auto const tile = room.tiles.get(x, y);

    if(tile == 9)
    {
      auto const pos = Vector(x, y);
      room.spawners.push_back({ 0, pos, "spikes" });
      room.tiles.set(x, y, 0);
    }
    else if(tile == 10)
    {
      auto const pos = Vector(x, y);
      room.spawners.push_back({ 0, pos, "ladder" });
      room.tiles.set(x, y, 0);
    }
  }
}

static void removeVersion(string& data)
{
  auto i = data.find("\"version\":");

  if(i == string::npos)
    return; // nothing to do

  auto j = i;

  while(data[j] != ',')
    ++j;

  data.erase(i, j - i + 1);
}

static Vec2i operator * (Vec2i a, Vec2i b)
{
  return { a.x * b.x, a.y * b.y };
}

static
Room loadAbstractRoom(json::Value const& jsonRoom)
{
  const int WorldMaxHeight = CELL_SIZE.y * 50;
  auto box = getRect(jsonRoom);
  box = transformTiledRect(box, WorldMaxHeight);
  box.pos.x /= CELL_SIZE.x;
  box.pos.y /= CELL_SIZE.y;
  box.size.x /= CELL_SIZE.x;
  box.size.y /= CELL_SIZE.y;

  auto const sizeInTiles = box.size * CELL_SIZE;

  auto const path = "assets/" + (string)jsonRoom["fileName"];
  auto const base = removeExtension(baseName(path));

  Room room;
  room.name = string(base.data, base.len);
  room.pos = box.pos;
  room.size = box.size;
  room.start = Vec2i(sizeInTiles.x / 2, sizeInTiles.y / 4);
  room.theme = 3;// atoi(string(jsonRoom["type"]).c_str());

  if(File::exists(path))
  {
    auto data = File::read(path);
    removeVersion(data);
    auto jsRoom = json::parse(data.c_str(), data.size());
    loadConcreteRoom(room, jsRoom);
  }
  else
  {
    generateConcreteRoom(room);
  }

  auto const actualSize = room.tiles.size;

  if(actualSize != room.size * CELL_SIZE)
  {
    char buffer[256];
    String s;
    s.data = buffer;
    s.len = sprintf(buffer, "room instance at (%d;%d) with theme %s/%d has wrong dimensions: map expected %dx%d, but the concrete tileset is %dx%d\n",
                    room.pos.x, room.pos.y,
                    path.c_str(), room.theme,
                    room.size.x * CELL_SIZE.x, room.size.y * CELL_SIZE.y,
                    actualSize.x, actualSize.y);
    throw Error(s);
  }

  return room;
}

Quest loadTiledWorld(string path) // tiled JSON ".world" format
{
  auto data = File::read(path);
  removeVersion(data);
  auto js = json::parse(data.c_str(), data.size());

  Quest r;

  for(auto& roomValue : js["maps"].elements)
  {
    auto room = loadAbstractRoom(roomValue);
    r.rooms.push_back(std::move(room));
  }

  return r;
}

Matrix2<int> parseMatrix(Vec2i size, String s)
{
  auto parseInteger = [&] () -> int
    {
      while(s[0] == ' ')
        s += 1;

      assert(s.len > 0);
      int result = 0;
      int sign = 1;

      if(s[0] == '-')
      {
        sign = -1;
        s += 1;
      }

      while(s[0] >= '0' && s[0] <= '9')
      {
        result *= 10;
        result += s[0] - '0';
        s += 1;
      }

      assert(s[0] == ',');
      s += 1;

      return result * sign;
    };

  Matrix2<int> r(size);

  for(int row = 0; row < r.size.y; ++row)
  {
    for(int col = 0; col < r.size.x; ++col)
    {
      r.set(col, row, parseInteger());
    }
  }

  return r;
}

Quest loadQuest(string path)
{
  auto compressedData = File::read(path);

  auto data = gzipDecompress({ (uint8_t*)compressedData.c_str(), (int)compressedData.size() });
  auto js = json::parse((const char*)data.data(), data.size());

  auto layers = getAllLayers(js);

  auto layer = layers["rooms"];

  Quest r;

  for(auto& jsonRoom : layer["objects"].elements)
  {
    Room room {};
    room.name = string(jsonRoom["name"]);
    room.theme = atoi(string(jsonRoom["type"]).c_str());
    room.pos.x = int(jsonRoom["x"]);
    room.pos.y = int(jsonRoom["y"]);
    room.start.x = int(jsonRoom["start_x"]);
    room.start.y = int(jsonRoom["start_y"]);
    room.size.x = int(jsonRoom["width"]);
    room.size.y = int(jsonRoom["height"]);
    room.tiles = parseMatrix(room.size * CELL_SIZE, std::string(jsonRoom["tiles"]));
    room.tilesForDisplay = parseMatrix(room.size * CELL_SIZE, std::string(jsonRoom["tilesForDisplay"]));

    for(auto& jsonSpawner : jsonRoom["entities"].elements)
    {
      Room::Spawner s;
      s.id = int(jsonSpawner["id"]);
      s.name = string(jsonSpawner["type"]);
      s.pos.x = double(int(jsonSpawner["x"])) / PRECISION;
      s.pos.y = double(int(jsonSpawner["y"])) / PRECISION;

      if(jsonSpawner.has("props"))
      {
        for(auto& member : jsonSpawner["props"].members)
          s.config[member.first] = string(member.second);
      }

      room.spawners.push_back(s);
    }

    r.rooms.push_back(std::move(room));
  }

  return r;
}

