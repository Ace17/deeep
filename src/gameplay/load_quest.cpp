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

#include "misc/decompress.h"
#include "misc/file.h"
#include "misc/json.h"
#include "misc/util.h" // baseName, removeExtension

#include "quest.h"

extern const Vec2i CELL_SIZE { 15, 10 };

namespace
{
Vec2i getSize(json::Value const& obj)
{
  return Vec2i(obj["width"], obj["height"]);
}

Rect2i getRect(json::Value const& obj)
{
  Rect2i r;

  r.pos = Vec2i((int)obj["px"].elements[0], (int)obj["px"].elements[1]);
  r.size = getSize(obj);

  return r;
}

const int TiledPixelsPerTile = 16;

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

std::map<std::string, json::Value> getAllLayers(json::Value const& js)
{
  std::map<std::string, json::Value> nameToLayer;

  for(auto& layer : js["layerInstances"].elements)
  {
    auto name = (std::string)layer["__identifier"];
    nameToLayer[name] = layer;
  }

  return nameToLayer;
}

std::vector<int> toIntArray(const json::Value& json)
{
  std::vector<int> r;
  r.reserve(json.elements.size());

  for(auto& e : json.elements)
    r.push_back((int)e);

  return r;
}

Matrix2<int> parseTileLayer(json::Value& json)
{
  Matrix2<int> tiles;

  auto const buff = toIntArray(json["intGridCsv"]);
  auto const size = Vec2i(json["__cWid"], json["__cHei"]);

  if(size.x * size.y != (int)buff.size())
    throw Error("invalid TMX file: width x height doesn't match data length");

  tiles.resize(size);

  for(auto pos : rasterScan(size.x, size.y))
  {
    auto const x = pos.first;
    auto const y = pos.second;

    int srcOffset = (x + (size.y - 1 - y) * size.x);
    int tile = buff[srcOffset];

    assert(tile >= 0);
    tiles.set(x, y, tile);
  }

  return tiles;
}

Matrix2<int> parseAutoLayerTiles(const json::Value& json)
{
  const auto size = Vec2i(json["__cWid"], json["__cHei"]);

  Matrix2<int> r(size);

  for(auto pos : rasterScan(size.x, size.y))
    r.set(pos.first, pos.second, -1);

  for(auto& t : json["autoLayerTiles"].elements)
  {
    Vec2i pos;
    pos.x = int(t["px"].elements[0]) / TiledPixelsPerTile;
    pos.y = size.y - 1 - int(t["px"].elements[1]) / TiledPixelsPerTile;

    const int tile = int(t["t"]);
    r.set(pos.x, pos.y, tile);
  }

  return r;
}

std::vector<Room::Spawner> parseThingLayer(json::Value const& objectLayer, int height)
{
  std::vector<Room::Spawner> r;

  for(auto& obj : objectLayer["entityInstances"].elements)
  {
    auto const objRect = transformTiledRect(getRect(obj), height);

    Room::Spawner spawner;

    spawner.pos = Vector(objRect.pos.x, objRect.pos.y);
    spawner.name = (std::string)obj["__identifier"];

    for(auto& c : spawner.name)
      c = std::tolower(c);

    spawner.config["width"] = std::to_string(objRect.size.x);
    spawner.config["height"] = std::to_string(objRect.size.y);

    if(obj.has("fieldInstances"))
    {
      for(auto& prop : obj["fieldInstances"].elements)
      {
        auto varName = (std::string)prop["__identifier"];
        auto varValue = std::to_string((int)prop["__value"]);
        spawner.config[varName] = varValue;
      }
    }

    r.push_back(spawner);
  }

  return r;
}

void loadConcreteRoom(Room& room, json::Value const& jsRoom)
{
  auto layers = getAllLayers(jsRoom);
  room.tiles = parseTileLayer(layers["IntGrid"]);
  room.tilesForDisplay = parseAutoLayerTiles(layers["IntGrid"]);

  if(exists(layers, "Entities"))
    room.spawners = parseThingLayer(layers["Entities"], room.size.y * CELL_SIZE.y);

  // process start point, if any
  for(auto& s : room.spawners)
  {
    if(s.name == "start")
    {
      auto i = int(&s - room.spawners.data());
      room.start = Vec2i(s.pos.x, s.pos.y);
      room.spawners.erase(room.spawners.begin() + i);
      break;
    }
  }
}

Vec2i operator * (Vec2i a, Vec2i b)
{
  return { a.x * b.x, a.y * b.y };
}

Room loadAbstractRoom(json::Value const& jsonRoom)
{
  const int WorldMaxHeight = CELL_SIZE.y * 50;

  Rect2i box;
  box.pos = Vec2i(jsonRoom["worldX"], jsonRoom["worldY"]);
  box.size = Vec2i(jsonRoom["pxWid"], jsonRoom["pxHei"]);

  box = transformTiledRect(box, WorldMaxHeight);
  box.pos.x /= CELL_SIZE.x;
  box.pos.y /= CELL_SIZE.y;
  box.size.x /= CELL_SIZE.x;
  box.size.y /= CELL_SIZE.y;

  auto const sizeInTiles = box.size * CELL_SIZE;

  auto const path = "assets/" + (std::string)jsonRoom["externalRelPath"];
  auto const base = removeExtension(baseName(path));

  Room room;
  room.name = std::string(base.data, base.len);
  room.pos = box.pos;
  room.size = box.size;
  room.start = Vec2i(sizeInTiles.x / 2, sizeInTiles.y / 4);
  room.theme = 3;// atoi(std::string(jsonRoom["type"]).c_str());

  {
    auto data = File::read(path);
    auto jsRoom = json::parse(data.c_str(), data.size());
    loadConcreteRoom(room, jsRoom);
  }

  auto const actualSize = room.tiles.size;

  if(actualSize != room.size * CELL_SIZE)
  {
    char buffer[256];
    String s = format(buffer, "room instance at (%d;%d) with theme %s/%d has wrong dimensions: map expected %dx%d, but the concrete tileset is %dx%d\n",
                      room.pos.x, room.pos.y,
                      path.c_str(), room.theme,
                      room.size.x * CELL_SIZE.x, room.size.y * CELL_SIZE.y,
                      actualSize.x, actualSize.y);
    throw Error(s);
  }

  return room;
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
}

Quest loadTiledWorld(std::string path) // LDTK JSON format
{
  auto data = File::read(path);
  auto js = json::parse(data.c_str(), data.size());

  Quest r;

  for(auto& roomValue : js["levels"].elements)
  {
    auto room = loadAbstractRoom(roomValue);
    r.rooms.push_back(std::move(room));
  }

  return r;
}

Quest loadQuest(std::string path)
{
  auto compressedData = File::read(path);

  auto data = gzipDecompress({ (uint8_t*)compressedData.c_str(), (int)compressedData.size() });
  auto js = json::parse((const char*)data.data(), data.size());

  Quest r;

  for(auto& jsonRoom : js["rooms"].elements)
  {
    Room room {};
    room.name = std::string(jsonRoom["name"]);
    room.theme = atoi(std::string(jsonRoom["type"]).c_str());
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
      s.name = std::string(jsonSpawner["type"]);
      s.pos.x = double(int(jsonSpawner["x"])) / PRECISION;
      s.pos.y = double(int(jsonSpawner["y"])) / PRECISION;

      if(jsonSpawner.has("props"))
      {
        for(auto& member : jsonSpawner["props"].members)
          s.config[member.first] = std::string(member.second);
      }

      room.spawners.push_back(s);
    }

    r.rooms.push_back(std::move(room));
  }

  return r;
}

