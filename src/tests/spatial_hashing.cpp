// Copyright (C) 2025 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "gameplay/spatial_hashing.h"
#include "tests.h"

unittest("Spatial hashing: one object")
{
  HashedSpace hs;

  hs.putObject(Rect2f({ 8, 6 }, { 1, 1 }), 1234);

  assertEquals(1, (int)hs.getObjectsInRect(Rect2f({ 7, 5 }, { 3, 3 })).size());
}

unittest("Spatial hashing: two objects in the same bucket")
{
  HashedSpace hs;

  hs.putObject(Rect2f({ 8, 6 }, { 1, 1 }), 1234);
  hs.putObject(Rect2f({ 2, 2 }, { 1, 1 }), 5678);

  assertEquals(1, (int)hs.getObjectsInRect(Rect2f({ 7, 5 }, { 3, 3 })).size());
  assertEquals(0, (int)hs.getObjectsInRect(Rect2f({ 10, 50 }, { 3, 3 })).size());
}

unittest("Spatial hashing: two far away objects")
{
  HashedSpace hs;

  hs.putObject(Rect2f({ 8, 6 }, { 1, 1 }), 1234);
  hs.putObject(Rect2f({ 200, 2 }, { 1, 1 }), 5678);

  assertEquals(std::vector<uintptr_t>{ 1234 }, hs.getObjectsInRect(Rect2f({ 7, 5 }, { 3, 3 })));
  assertEquals(std::vector<uintptr_t>{ 5678 }, hs.getObjectsInRect(Rect2f({ 199, 0 }, { 3, 3 })));
}

unittest("Spatial hashing: object spanning on multiple cells")
{
  HashedSpace hs;

  hs.putObject(Rect2f({ 10, 10 }, { 100, 20 }), 1234);

  assertEquals(std::vector<uintptr_t>{ 1234 }, hs.getObjectsInRect(Rect2f({ 50, 15 }, { 1, 1 })));
  assertEquals(std::vector<uintptr_t>{ 1234 }, hs.getObjectsInRect(Rect2f({ 70, 15 }, { 1, 1 })));
  assertEquals(std::vector<uintptr_t>{ 1234 }, hs.getObjectsInRect(Rect2f({ 90, 15 }, { 1, 1 })));
  assertEquals(std::vector<uintptr_t>{}, hs.getObjectsInRect(Rect2f({ 0, 0 }, { 1, 1 })));

  hs.removeObject(Rect2f({ 10, 10 }, { 100, 20 }), 1234);
  assertEquals(std::vector<uintptr_t>{}, hs.getObjectsInRect(Rect2f({ 70, 15 }, { 1, 1 })));
}

