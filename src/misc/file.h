// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "base/span.h"
#include "base/string.h"
#include <string>
using namespace std;

namespace File
{
string read(String path);
void write(String path, Span<const uint8_t> data);
bool exists(String path);
}

