// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "misc/base64.h"
#include "tests.h"
#include <vector>
using namespace std;

unittest("Base64: simple")
{
  assertEquals(vector<uint8_t>({}), decodeBase64(""));
  assertEquals(vector<uint8_t>({ 'a' }), decodeBase64("YQ=="));
  assertEquals(vector<uint8_t>({ 'C', 'o', 'o', 'l' }), decodeBase64("Q29vbA=="));
  assertEquals(vector<uint8_t>({ 'H', 'e', 'l', 'l', 'o' }), decodeBase64("SGVsbG8="));
}

