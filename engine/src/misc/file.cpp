// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "file.h"

#include <cstdio>
#include <stdexcept>

using namespace std;

namespace File
{
string read(string path)
{
  FILE* fp = fopen(path.c_str(), "rb");

  if(!fp)
    throw runtime_error("Can't open file '" + path + "' for reading");

  fseek(fp, 0, SEEK_END);
  auto size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  string r;
  r.resize(size);
  fread(&r[0], 1, r.size(), fp);

  fclose(fp);

  return r;
}

void write(string path, Span<const uint8_t> data)
{
  FILE* fp = fopen(path.c_str(), "wb");

  if(!fp)
    throw runtime_error("Can't open file '" + path + "' for writing");

  fwrite(data.data, 1, data.len, fp);
  fflush(fp);
  fclose(fp);
}

bool exists(string path)
{
  FILE* fp = fopen(path.c_str(), "rb");

  if(!fp)
    return false;

  fclose(fp);
  return true;
}
}

