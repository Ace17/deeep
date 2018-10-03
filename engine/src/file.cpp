// Copyright (C) 2018 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#include "file.h"

#include <fstream>

inline
ifstream openInput(string path)
{
  ifstream fp(path, ios::binary);

  if(!fp.is_open())
    throw runtime_error("Can't open file '" + path + "'");

  return fp;
}

string read(string path)
{
  auto fp = openInput(path);

  fp.seekg(0, ios::end);
  auto size = fp.tellg();
  fp.seekg(0, ios::beg);

  string r;
  r.resize(size);
  fp.read(&r[0], r.size());

  return r;
}

bool exists(string path)
{
  return ifstream(path).is_open();
}

