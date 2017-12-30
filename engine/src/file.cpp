/*
 * Copyright (C) 2017 - Sebastien Alaiwan <sebastien.alaiwan@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "file.h"

#include <fstream>
#include <sstream>

inline
ifstream openInput(string path)
{
  ifstream fp(path);

  if(!fp.is_open())
    throw runtime_error("Can't open file '" + path + "'");

  return fp;
}

string read(string path)
{
  auto fp = openInput(path);

  string r;
  string line;

  while(getline(fp, line))
    r += line;

  return r;
}

bool exists(string path)
{
  return ifstream(path).is_open();
}

