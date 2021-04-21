// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// Tiny base64 decoder

#include "base64.h"

#include "base/error.h"

static const string base64_map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

vector<uint8_t> decodeBase64(string const& input)
{
  if(input.length() % 4 != 0)
    throw Error("invalid base64 input");

  vector<uint8_t> decoded;

  for(int i = 0; i < (int)input.size(); i += 4)
  {
    int quad[4];
    quad[0] = base64_map.find(input[i + 0]);
    quad[1] = base64_map.find(input[i + 1]);
    quad[2] = base64_map.find(input[i + 2]);
    quad[3] = base64_map.find(input[i + 3]);

    decoded.push_back((quad[0] << 2) | (quad[1] >> 4));

    if(quad[2] == 64)
      continue;

    decoded.push_back((quad[1] << 4) | (quad[2] >> 2));

    if(quad[3] == 64)
      continue;

    decoded.push_back((quad[2] << 6) | quad[3]);
  }

  return decoded;
}

